#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
diode1-client.py: Synthetic diode #1

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

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

def recvmsg(conn):
    """ Receive a protobuf message msg, preceded by a 4-bit specification of the size
    
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
    msg = conn.recv(msgsize)
        # TODO: Error handling for socket closed
        # raise Exception("Socket closed.")
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
    diode.attrs["trace_xvals"] = settings.trace_xvals
    diode.attrs["trace_nx"] = settings.trace_nx
    
    trace = diode.create_group("Trace")
    trace.create_dataset("shot_num", (nt_max,), dtype=np.uint16, chunks=True) # TODO: Smarter chunking
    trace.create_dataset("yvals", (nt_max, settings.trace_nx), dtype=np.uint16, chunks=True) # TODO: Smarter chunking
    
    metrics = diode.create_group("Metrics")
    metrics.create_dataset("shot_num", (nm_max,), dtype=np.uint64, chunks=True)
    metrics.create_dataset("trace_sum", (nm_max,), dtype=np.uint64, chunks=True)
    metrics.create_dataset("trace_mean", (nm_max,), dtype=np.float64, chunks=True)
    metrics.create_dataset("trace_max", (nm_max,), dtype=np.uint64, chunks=True)
    metrics.create_dataset("trace_min", (nm_max,), dtype=np.float64, chunks=True)
    metrics.create_dataset("trace_custom", (nm_max,), dtype=np.uint64, chunks=True)
    
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
    metrics_h5["trace_sum"][j:j + nm] = metrics_pbuf.trace_sum
    metrics_h5["trace_mean"][j:j + nm] = metrics_pbuf.trace_mean
    metrics_h5["trace_max"][j:j + nm] = metrics_pbuf.trace_max
    metrics_h5["trace_min"][j:j + nm] = metrics_pbuf.trace_min
    metrics_h5["trace_custom"][j:j + nm] = metrics_pbuf.trace_custom

def resize_trace_h5(trace_h5, nshots):
    """ Resize the HDF5 arrays related to diode traces, truncating or expanding to "nshots" shots """
    for grpname in ["shot_num", "yvals"]:
        trace_h5[grpname].resize(nshots, axis=0)

def resize_metrics_h5(metrics_h5, nshots):
    """ Resize the HDF5 arrays related to diode metrics, truncating or expanding to "nshots" shots """
    for grpname in ["shot_num", "trace_sum", "trace_mean", "trace_max", "trace_min", "trace_custom"]:
        metrics_h5[grpname].resize(nshots, axis=0)

# TODO: Organize into setup, daq, cleanup
def connection_callback(conn, addr):
    """ Callback to be called in its own thread on any new connection """
    with conn:
        print('Connected by', addr)
        
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
                    
            nt_max = 10000 # Max number of shots with traces to store in a single HDF5 file
            nm_max = 10000 # Max number of shots with metrics to store in a single HDF5 file
            
            while msg:
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
                    print("File closed.")
                
if __name__ == "__main__":
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        start = time.time()
        connection_callback(conn, addr)
        end = time.time()
        print(end - start)
        s.close()
                