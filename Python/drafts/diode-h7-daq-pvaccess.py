#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
diode-h7-daq-pvaccess.py: Takes the DAQ stream from a Nucleo and puts it onto pvaccess.


TODO:
* On Nucleo firmware, toss in an "await_update" if trigcnt is changed. (Might not solve everything, but better than nothing).

Created by Scott Feister on March 22, 2024.
"""

import sys
sys.path.append("../proto")

import os
import time
import socket
import numpy as np

from p4p.nt import NTScalar, NTNDArray
from p4p.server import Server
from p4p.server.thread import SharedPV

from datetime import datetime
from flsuite import sftools as sf
from compiled_python import hello_pb2, diode_pb2
try:
    import Adafruit_BBIO.GPIO as GPIO  # for flashing LED indicator on Beaglebone Black
    bbio = True
except:
    bbio = False
    
HOST = '196.254.31.201'  # The remote device's hostname or IP address
PORT = 1234        # The port used by the remote device for DAQ transfer
PIN_LED = "P8_7" # for flashing LED indicator on Beaglebone Black

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

def resize_trace_h5(trace_h5, nshots):
    """ Resize the HDF5 arrays related to diode traces, truncating or expanding to "nshots" shots """
    for grpname in ["shot_num", "shot_time_seconds", "shot_time_nanos", "yvals"]:
        trace_h5[grpname].resize(nshots, axis=0)

def resize_metrics_h5(metrics_h5, nshots):
    """ Resize the HDF5 arrays related to diode metrics, truncating or expanding to "nshots" shots """
    for grpname in ["shot_num", "shot_time_seconds", "shot_time_nanos", "shot_time_alt_nanos", "bg_mean", "reduced_mean"]:
        metrics_h5[grpname].resize(nshots, axis=0)

# TODO: Organize into setup, daq, cleanup
def connection_callback(conn):
    """ Callback to be called in its own thread on any new connection """
    # TODO: Handle breaking out of this process where needed, if recv returns zero
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
                
        while msg:
            start = time.time()
            # Go endlessly in the following loop, until TCP connection is closed.

            diode_h5, trace_h5, metrics_h5 = init_diode_h5(f, hello, settings, nt_max, nm_max)
        
            i = 0 # Trace index
            j = 0 # Metrics index
            nm = 0 # Number of metrics per packet
            
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
                    if bbio:
                        GPIO.output(PIN_LED, not GPIO.input(PIN_LED)) # toggle LED
                    insert_metrics_h5(metrics_h5, data.metrics, j)
                    j += nm
                    
            del data # TODO: Is this needed? Check for any memory leaks.
            
            print("File closed.")
            end = time.time()
            print("Time elapsed for batch: ", end - start)
            if bbio:
                GPIO.output(PIN_LED, GPIO.LOW)
                
if __name__ == "__main__":
    if bbio:
        GPIO.setup(PIN_LED, GPIO.OUT) # for flashing LED indicator on Beaglebone Black
    
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
        finally:
            if bbio:
                GPIO.output(PIN_LED, GPIO.LOW)
            
        time.sleep(10);

        
        
        # TODO: Catch timeouts, errors of all kinds, etc!

                