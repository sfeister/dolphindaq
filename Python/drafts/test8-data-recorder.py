#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
test8-data-recorder.py: Attempt to explore a data recorder


TODO:
* Add TCP keepalive-style timeouts in case remote device wire is disconnected (so that HDF5 is closed). Currently hangs indefinitely until I do Ctrl + C.
* On Nucleo firmware, toss in an "await_update" if trigcnt is changed. (Might not solve everything, but better than nothing).

Created by Scott Feister on Thu Jan 21 20:04:13 2021
"""

import sys
sys.path.append("../../proto")

import os
import time
import socket
import numpy as np
import h5py

from datetime import datetime
from flsuite import sftools as sf
from compiled_python import hello_pb2, diode_pb2

HOST = '192.168.203.151'  # The remote device's hostname or IP address
PORT = 1234        # The port used by the remote device for DAQ transfer

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
    diode.attrs["unique_name"] = hello.unique_name
    diode.attrs["unique_id"] = hello.unique_id
    diode.attrs["device_type"] = hello.device_type
    
    diode.attrs["shot_num_init"] = settings.shot_num
    diode.attrs["trace_dt"] = settings.trace_dt
    # TODO: Add other stuff here
    
    trace = diode.create_group("Trace")
    trace.create_dataset("shot_num", (nt_max,), maxshape=(nt_max,), dtype=np.uint64, chunks=True) # TODO: Smarter chunking
    trace.create_dataset("yvals", (nt_max, settings.trace_nt), maxshape=(nt_max, settings.trace_nt), dtype=np.uint16, chunks=True) # TODO: Smarter chunking
    
    metrics = diode.create_group("Metrics")
    metrics.create_dataset("shot_num", (nm_max,), dtype=np.uint64, chunks=True)
    metrics.create_dataset("bg_mean", (nm_max,), dtype=np.double, chunks=True)
    metrics.create_dataset("reduced_integ", (nm_max,), dtype=np.double, chunks=True)
    
    return diode, trace, metrics
                
def insert_trace_h5(trace_h5, trace_pbuf, i):
    """ Insert a protobuf trace message into the HDF5 file at index i """
    # TODO: Check that it will fit (?) Or perhaps just handle the error message
    trace_h5["shot_num"][i] = trace_pbuf.shot_num
    trace_h5["yvals"][i, :] = np.frombuffer(trace_pbuf.yvals, dtype=np.uint16)

def insert_metrics_h5(metrics_h5, metrics_pbuf, j):
    """ Insert a set of protobuf diode metrics message into the HDF5 file at index j """
    nm = len(metrics_pbuf.shot_num)
    
    # TODO: Check that it will fit (?) Or perhaps just handle the error message
    metrics_h5["shot_num"][j:j + nm] = metrics_pbuf.shot_num
    metrics_h5["bg_mean"][j:j + nm] = metrics_pbuf.bg_mean
    metrics_h5["reduced_integ"][j:j + nm] = metrics_pbuf.reduced_integ

def resize_trace_h5(trace_h5, nshots):
    """ Resize the HDF5 arrays related to diode traces, truncating or expanding to "nshots" shots """
    for grpname in ["shot_num", "yvals"]:
        trace_h5[grpname].resize(nshots, axis=0)

def resize_metrics_h5(metrics_h5, nshots):
    """ Resize the HDF5 arrays related to diode metrics, truncating or expanding to "nshots" shots """
    for grpname in ["shot_num", "bg_mean", "reduced_integ"]:
        metrics_h5[grpname].resize(nshots, axis=0)

# TODO: Organize into setup, daq, cleanup
def connection_callback(conn):
    """ Callback to be called in its own thread on any new connection """
    # TODO: Handle breaking out of this process where needed, if recv returns zero
    msg = recvmsg(conn)
    if not msg:
        return  # HDF5 file isn't yet open. Break out of function immediately if TCP connection closed here.
    hello = hello_pb2.Hello()
    hello.ParseFromString(msg)
    print(hello)

    # Handle the Hello message
    if hello.device_type == hello_pb2.Hello.DIODE:
        msg = recvmsg(conn)
        if not msg:
            return # HDF5 file isn't yet open. Break out of function immediately if TCP connection closed here.
        settings = diode_pb2.Settings()
        settings.ParseFromString(msg)
        print(settings)
                
        nt_max = 1000 # Max number of shots with traces to store in a single HDF5 file (10k ~= 80 MB. Trace settings on device will set the file size.) 
        nm_max = 60000 # Max number of shots with metrics to store in a single HDF5 file (60k = 60 seconds. Metrics settings here will set the batch time duration.)
        
        while msg:
            start = time.time()
            # Go endlessly in the following loop, until TCP connection is closed.
            now = datetime.now() # current date and time
            outdir = sf.subdir2("out", now.strftime("%Y-%m-%d")) # Directory of today's date
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
                
                while (i < nt_max) and (j < nm_max - nm):
                    # TCP receipt of data
                    msg = recvmsg(conn)
                    if not msg:
                        break
                    data = diode_pb2.Data()
                    data.ParseFromString(msg)
    
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
                print("J is, ", j)
                print("File closed.")
                end = time.time()
                print("Time elapsed for batch: ", end - start)
                
if __name__ == "__main__":
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        #s.settimeout(4.0) # timeout is in seconds
        s.connect((HOST, PORT))
        connection_callback(s)
        s.close()
        
        # TODO: Catch timeouts, errors of all kinds, etc!

                