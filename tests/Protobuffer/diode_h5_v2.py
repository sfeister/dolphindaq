#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
diode_h5.py: HDF5 code for saving "Diode" class protobuffers to file

Created by Scott Feister on Sat Jan 23 15:13:01 2021
"""

import sys
sys.path.append("../../proto")

import h5py
import numpy as np
import os
from datetime import datetime
from compiled_python import hello_pb2, diode_pb2
from synthetic2 import Diode
from flsuite import sftools as sf

def receive_synth_hello():
    hello = hello_pb2.Hello()
    hello.unique_name = "Diode1"
    hello.unique_id = 500
    hello.device_type = hello_pb2.Hello.DIODE
    return hello

def receive_synth_diode_settings():
    """ Synthetic receipt of data """
    settings = diode_pb2.Settings()
    xmin = 1.0e5
    xmax = 1.0e10
    nx = 800
    
    settings.shot_num = 0
    settings.trace_xvals[:] = np.linspace(xmin, xmax, nx)
    settings.trace_nx = nx
    
    return settings

def receive_synth_diode_data():
    """ Synthetic receipt of data """
    nx = 800
    data = diode_pb2.Data()
    data.trace.shot_num = 5
    d1 = Diode(nshots=10, nx=nx)
    data.trace.yvals = d1.getshot(5).tobytes()
    
    data.metrics.shot_num[:] = np.arange(25)
    data.metrics.trace_sum[:] = np.arange(25)*10
    data.metrics.trace_mean[:] = np.arange(25)*10
    data.metrics.trace_max[:] = np.arange(25)*10
    data.metrics.trace_min[:] = np.arange(25)*10
    data.metrics.trace_custom[:] = np.arange(25)*10
    
    # To be more general, I should consider allowing multiple traces! Then I could pack them in bunches... but probably fine for now.
    return data


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

if __name__ == "__main__":
 
    hello = receive_synth_hello()
        
    # Handle the message!
    if hello.device_type == hello_pb2.Hello.DIODE:
        settings = receive_synth_diode_settings()
        
        for k in range(10):
            # Go endlessly in the following loop, until TCP connection is closed.
            now = datetime.now() # current date and time
            outdir = sf.subdir2("out", now.strftime("%Y-%m-%d")) # Directory of today's date
            h5fn = os.path.join(outdir, hello.unique_name + "-" + now.strftime("%Y-%m-%d_%Hh%Mm%Ss%f")[:-3] + "ms.h5") # HDF5 filename
            print("Opening file: " + h5fn)
            with h5py.File(h5fn, 'w') as f: # Need a new one every time settings change
                nt_max = 100 # Max number of shots with traces to store in a single HDF5 file
                nm_max = 10000 # Max number of shots with metrics to store in a single HDF5 file
                
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
                    data = receive_synth_diode_data()
    
                    # Store the data message appropriately
                    if data.HasField("trace"):
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
                            
                    del data
                
                f.attrs["File Closure Time"] = datetime.now().strftime(ftimespec)
                resize_trace_h5(trace_h5, i)
                resize_metrics_h5(metrics_h5, j)
            print("File closed.")

    else:
        print("Device type not recognized.")
        # TODO: Close TCP connection