#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
diode-h7-pva-data-recorder.py: A data recorder for the DIODE-H7 device, with PVAccess.

Example Command Line Usage:
    python diode-h7-pva-data-recorder.py -p "DIODE-H7-01" -n "196.254.31.201" -o "/home/debian/data"

TODO:
* Add TCP keepalive-style timeouts in case remote device wire is disconnected (so that HDF5 is closed). Currently hangs indefinitely until I do Ctrl + C.
* On Nucleo firmware, toss in an "await_update" if trigcnt is changed. (Might not solve everything, but better than nothing).

Created by Scott Feister on in July 2022, then updated in March 28, 2024 for PVAccess.
"""

import sys
sys.path.append("../proto")

import os
import time
import socket
import numpy as np
import h5py

from datetime import datetime
from compiled_python import hello_pb2, diode_pb2

from p4p.nt import NTScalar, NTNDArray
from p4p.server import Server
from p4p.server.thread import SharedPV
from p4p.client.thread import Context

import argparse

# Argument parsing with help from: https://stackoverflow.com/questions/40710719/optional-command-line-arguments
parser = argparse.ArgumentParser(description = "Description for my parser")
parser.add_argument("-p", "--prefix", help = 'Specify the EPICS prefix we will give to this device. Example: --prefix "DIODE-H7-01"', required = True)
parser.add_argument("-n", "--nucleo", help = 'Specify the IP Address of the Nucleo to which we will connect. Example: --nucleo "196.254.31.201"', required = True)
parser.add_argument("-o", "--output", help = 'Specify the output folder in which to save traces. This folder must already exist; it will not be created. Example: --output "/home/debian/data"', required=False, default="/home/debian/data")

argument = parser.parse_args()
DEVICE_NAME = str(argument.prefix)
HOST = str(argument.nucleo)  # The remote device's hostname or IP address
DATADIR = str(argument.output) # this folder should already exist on this computer, and is where data will be stored

PORT = 1234        # The port used by the remote device for DAQ transfer

def sf_subdir(folder, name):
    """ Make a subdirectory in the specified folder, if it doesn't already exist
    Copied from flsuite.sftools.subdir() to reduce the dependency count!
    """
    subpath = os.path.join(folder,name)
    if not os.path.exists(subpath):
        try:
            os.mkdir(subpath)
        except:
            if not os.path.exists(subpath):
                raise
    return subpath
    
def recvmsg(conn):
    """ Receive a protobuf message msg, preceded by a 4-byte specification of the size
    
    Inputs:
        conn    Already-accepted TCP socket connection
        
    Outputs:
        msg     Protobuf message object
    """
    msgsize_b = conn.recv(4)
    if not msgsize_b:
        return None
        # TODO: Error handling for socket closed
        # raise Exception("Socket closed.")
    msgsize = np.frombuffer(msgsize_b, np.uint32)[0] # ugly, convert from bytes to uint32
    
    msg = bytes();
    while len(msg) < msgsize:
        msg_tmp = conn.recv(msgsize - len(msg)) # TODO: add a loop to continue if all bytes haven't arrived yet
        if not msg_tmp:
            return None
            # TODO: Error handling for socket closed
            # raise Exception("Socket closed.")
        else:
            msg += msg_tmp
    return msg

def init_diode_h5(f, hello, settings, nt_max, nm_max):
    """ Initialize the diode HDF5 file 
    Inputs:
        f: Open, empty HDF5 file
        hello: Protobuf object from hello message
        settings: Protobuf object from diode settings message
    Outputs:
        IO on diode.hdf5
        diode: HDF5 handle to the diode category
        trace: HDF5 handle to the trace category (sub-category of diode)
        metrics: HDF5 handle to the metrics category (sub-category of diode)
    """
    
    diode = f.create_group(hello.unique_name)
    
    # TODO: Add versioning of protobufs

    # "Hello" message contents
    diode.attrs["unique_name"] = hello.unique_name
    diode.attrs["unique_id"] = hello.unique_id
    diode.attrs["device_type"] = hello.device_type
    
    # "Settings" message contents
    diode.attrs["start_shot_num"] = settings.start_shot_num
    diode.attrs["start_time_seconds"] = settings.start_time.seconds
    diode.attrs["start_time_nanos"] = settings.start_time.nanos
    diode.attrs["timtick_secs"] = settings.timtick_secs
    diode.attrs["dt"] = settings.dt
    diode.attrs["trace_dt"] = settings.trace_dt
    diode.attrs["trace_nt"] = settings.trace_nt
    diode.attrs["metrics_batch_size"] = settings.metrics_batch_size
    diode.attrs["trace_ymin"] = settings.trace_ymin
    diode.attrs["trace_ymax"] = settings.trace_ymax
    diode.attrs["t1"] = settings.t1
    diode.attrs["t1_dts"] = settings.t1_dts
    diode.attrs["t2"] = settings.t2
    diode.attrs["t2_dts"] = settings.t2_dts
    diode.attrs["t3"] = settings.t3
    diode.attrs["t3_dts"] = settings.t3_dts
    diode.attrs["t4"] = settings.t4
    diode.attrs["t4_dts"] = settings.t4_dts
    
    trace = diode.create_group("Trace")
    trace.create_dataset("shot_num", (nt_max,), maxshape=(nt_max,), dtype=np.uint64, chunks=True) # TODO: Smarter chunking
    trace.create_dataset("shot_time_seconds", (nt_max,), dtype=np.int64, chunks=True)
    trace.create_dataset("shot_time_nanos", (nt_max,), dtype=np.int32, chunks=True)

    trace.create_dataset("yvals", (nt_max, settings.trace_nt), maxshape=(nt_max, settings.trace_nt), dtype=np.uint16, chunks=True) # TODO: Smarter chunking
    
    metrics = diode.create_group("Metrics")
    metrics.create_dataset("shot_num", (nm_max,), dtype=np.uint64, chunks=True)
    metrics.create_dataset("shot_time_seconds", (nm_max,), dtype=np.int64, chunks=True)
    metrics.create_dataset("shot_time_nanos", (nm_max,), dtype=np.int32, chunks=True)
    metrics.create_dataset("shot_time_alt_nanos", (nm_max,), dtype=np.uint64, chunks=True)
    metrics.create_dataset("bg_mean", (nm_max,), dtype=np.double, chunks=True)
    metrics.create_dataset("reduced_mean", (nm_max,), dtype=np.double, chunks=True)
    metrics.create_dataset("reduced_max", (nm_max,), dtype=np.double, chunks=True)
    
    return diode, trace, metrics
                
def insert_trace_h5(trace_h5, trace_pbuf, i):
    """ Insert a protobuf trace message into the HDF5 file at index i """
    # TODO: Check that it will fit (?) Or perhaps just handle the error message
    trace_h5["shot_num"][i] = trace_pbuf.shot_num
    trace_h5["shot_time_seconds"][i] = trace_pbuf.shot_time.seconds
    trace_h5["shot_time_nanos"][i] = trace_pbuf.shot_time.nanos
    trace_h5["yvals"][i, :] = np.frombuffer(trace_pbuf.yvals, dtype=np.uint16)

def insert_metrics_h5(metrics_h5, metrics_pbuf, j):
    """ Insert a set of protobuf diode metrics message into the HDF5 file at index j """
    nm = len(metrics_pbuf.shot_num)
    
    # TODO: Check that it will fit (?) Or perhaps just handle the error message
    metrics_h5["shot_num"][j:j + nm] = metrics_pbuf.shot_num
    metrics_h5["shot_time_seconds"][j:j + nm] = metrics_pbuf.shot_time_seconds
    metrics_h5["shot_time_nanos"][j:j + nm] = metrics_pbuf.shot_time_nanos
    metrics_h5["shot_time_alt_nanos"][j:j + nm] = metrics_pbuf.shot_time_alt_nanos
    metrics_h5["bg_mean"][j:j + nm] = metrics_pbuf.bg_mean
    metrics_h5["reduced_mean"][j:j + nm] = metrics_pbuf.reduced_mean
    metrics_h5["reduced_max"][j:j + nm] = metrics_pbuf.reduced_max

def resize_trace_h5(trace_h5, nshots):
    """ Resize the HDF5 arrays related to diode traces, truncating or expanding to "nshots" shots """
    for grpname in ["shot_num", "shot_time_seconds", "shot_time_nanos", "yvals"]:
        trace_h5[grpname].resize(nshots, axis=0)

def resize_metrics_h5(metrics_h5, nshots):
    """ Resize the HDF5 arrays related to diode metrics, truncating or expanding to "nshots" shots """
    for grpname in ["shot_num", "shot_time_seconds", "shot_time_nanos", "shot_time_alt_nanos", "bg_mean", "reduced_mean", "reduced_max"]:
        metrics_h5[grpname].resize(nshots, axis=0)

# TODO: Organize into setup, daq, cleanup
def connection_callback(conn):
    """ Callback to be called in its own thread on any new connection """
    # TODO: Handle breaking out of this process where needed, if recv returns zero

    global dev_rec_on # PVA
    
    msg = recvmsg(conn)
    if not msg:
        print("No hello message.")
        return  # HDF5 file isn't yet open. Break out of function immediately if TCP connection closed here.
    hello = hello_pb2.Hello()
    hello.ParseFromString(msg)
    print(hello)

    # Handle the Hello message
    if hello.device_type == hello_pb2.Hello.DIODE:
        msg = recvmsg(conn)
        if not msg:
           print("No diode message.")
           return # HDF5 file isn't yet open. Break out of function immediately if TCP connection closed here.
        settings = diode_pb2.Settings()
        settings.ParseFromString(msg)
        print(settings)
                
        nt_max = 1000 # Max number of shots with traces to store in a single HDF5 file (10k ~= 80 MB. Trace settings on device will set the file size.) 
        nm_max = 30000 # Max number of shots with metrics to store in a single HDF5 file (60k = 60 seconds. Metrics settings here will set the batch time duration.)
        
        while msg:
            if dev_rec_on.current():
                start = time.time()
                # Go endlessly in the following loop, until TCP connection is closed.
                now = datetime.now() # current date and time
                outdir = sf_subdir(DATADIR, now.strftime("%Y-%m-%d")) # Directory of today's date
                h5fn = os.path.join(outdir, hello.unique_name + "-" + now.strftime("%Y-%m-%d_%Hh%Mm%Ss%f")[:-3] + "ms.h5") # HDF5 filename
                print("Opening file: " + h5fn)
                with h5py.File(h5fn, 'w') as f: # Need a new one every time settings change
                    ftimespec = r"%Y-%m-%d %H:%M:%S.%f"
                    f.attrs["Time Format"] = ftimespec
                    f.attrs["File Creation Time"] = now.strftime(ftimespec)
                    f.attrs["Original Filename"] = h5fn
                    # TODO: Put this analysis script in the file as well? IDK.
        
                    diode_h5, trace_h5, metrics_h5 = init_diode_h5(f, hello, settings, nt_max, nm_max)
                
                    i = 0 # Trace index
                    j = 0 # Metrics index
                    nm = 0 # Number of metrics per packet
                    
                    while dev_rec_on.current() and (i < nt_max) and (j < nm_max - nm):
                        # TCP receipt of data
                        try:
                            msg = recvmsg(conn)
                        except socket.timeout:
                            print(datetime.now())
                            print("Socket timeout.")
                            msg = None;
                            break
                        if not msg:
                            print(datetime.now())
                            print("Socket closed.")
                            break
                        data = diode_pb2.Data()
                        data.ParseFromString(msg)
                        
                        # Do anything pva-style with this data in EPICS
                        pva_handle_data(data)
        
                        # Store the data message appropriately
                        if data.HasField("trace"):
                            if data.trace.shot_num % 10 == 0:   # DEBUG ONLY
                                print(data.trace.shot_num)      # DEBUG ONLY

                            insert_trace_h5(trace_h5, data.trace, i)
                            i += 1
                        
                        if data.HasField("metrics"):                           
                            nm = len(data.metrics.shot_num)
                            if j + nm > nm_max:
                                raise Warning("Metrics don't all fit in the HDF5 file! Dumping this entire batch of metrics.")
                                # TODO: Expand the HDF5 file to fit them, and then pinch it off (??)
                            else:
                                insert_metrics_h5(metrics_h5, data.metrics, j)
                                j += nm
                                
                        del data # TODO: Is this needed? Check for any memory leaks.
                    
                    f.attrs["File Closure Time"] = datetime.now().strftime(ftimespec)
                    resize_trace_h5(trace_h5, i)
                    resize_metrics_h5(metrics_h5, j)
                    print("File closed.")
                    end = time.time()
                    print("Time elapsed for batch: ", end - start)
            else: # Recording disabled
                while not dev_rec_on.current():
                    # TCP receipt of data
                    try:
                        msg = recvmsg(conn)
                    except socket.timeout:
                        print(datetime.now())
                        print("Socket timeout.")
                        msg = None;
                        break
                    if not msg:
                        print(datetime.now())
                        print("Socket closed.")
                        break
                    data = diode_pb2.Data()
                    data.ParseFromString(msg)
                    
                    # Do anything pva-style with this data in EPICS
                    pva_handle_data(data)

                    del data # TODO: Is this needed? Check for any memory leaks.
                    
def pva_handle_data(data):
    global dev_metrics_reduced_mean_avg # PVA
    global dev_metrics_reduced_mean_std # PVA

    if data.HasField("metrics"):
        dev_metrics_reduced_mean_avg.post(np.mean(data.metrics.reduced_mean), timestamp=time.time())
        dev_metrics_reduced_mean_std.post(np.std(data.metrics.reduced_mean), timestamp=time.time())
    
                
if __name__ == "__main__":

    ### INITIALIZE PVACCESS EPICS SERVER
    # https://mdavidsaver.github.io/p4p/server.html
    ctxt = Context('pva')
    dev_info = SharedPV(nt=NTScalar('s'), # scalar string
                  initial="Diode Multi-Function PVA Server, 20240328")      # setting initial value also open()'s
    dev_rec_on = SharedPV(nt=NTScalar('?'), # scalar boolean
                  initial=True)      # setting initial value also open()'s
    dev_metrics_reduced_mean_avg = SharedPV(nt=NTScalar('d'), # scalar double
                  initial=0.0)      # setting initial value also open()'s
    dev_metrics_reduced_mean_std = SharedPV(nt=NTScalar('d'), # scalar double
                  initial=0.0)      # setting initial value also open()'s

    @dev_metrics_reduced_mean_std.put
    def dev_metrics_reduced_mean_std_handle(pv, op):
        pv.post(op.value()) # just store and update subscribers
        op.done()

    @dev_metrics_reduced_mean_avg.put
    def dev_metrics_reduced_mean_avg_handle(pv, op):
        pv.post(op.value()) # just store and update subscribers
        op.done()

    @dev_rec_on.put
    def dev_rec_on_handle(pv, op):
        pv.post(op.value()) # just store and update subscribers
        op.done()
        
    providers = {
        DEVICE_NAME + ':info': dev_info,
        DEVICE_NAME + ':RECORDER:on': dev_rec_on,
        DEVICE_NAME + ':METRICS:reduced_mean:avg': dev_metrics_reduced_mean_avg,
        DEVICE_NAME + ':METRICS:reduced_mean:std': dev_metrics_reduced_mean_std,
    }

    with Server(providers=[providers]) as S:
        while True: ## run forever!
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                    s.settimeout(4.0) # timeout is in seconds
                    s.connect((HOST, PORT))
                    connection_callback(s)
                    s.close()
            except socket.timeout:
                print(datetime.now())
                print("Socket timeout.")
            except socket.error as e:
                print(datetime.now())
                print(e)
                
            time.sleep(10);

            
            
            # TODO: Catch timeouts, errors of all kinds, etc!

                    