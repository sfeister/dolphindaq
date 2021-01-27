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
from compiled_python import hello_pb2, diode_pb2
from synthetic2 import Diode

if __name__ == "__main__":
    hello = hello_pb2.Hello()
    settings = diode_pb2.Settings()
 
    hello.unique_name = "Diode1"
    hello.unique_id = 500
    hello.device_type = hello_pb2.Hello.DIODE

    xmin = 1.0e5
    xmax = 1.0e10
    nx = 800
    d1 = Diode(nshots=10, nx=nx)
    
    settings.shot_num = 0
    settings.trace_xvals[:] = np.linspace(xmin, xmax, nx)
    settings.trace_nx = nx

    nt_max = 11 # Max number of shots with traces to store in a single HDF5 file
    nm_max = 300 # Max number of shots with metrics to store in a single HDF5 file
        
    with h5py.File(os.path.join("out", hello.unique_name + ".h5"), 'w') as f: # Need a new one every time settings change
        if hello.device_type == hello_pb2.Hello.DIODE:
            diode = f.create_group(hello.unique_name)
            
            # TODO: Add versioning of protobufs            
            diode.attrs["unique_name"] = hello.unique_name
            diode.attrs["unique_id"] = hello.unique_id
            diode.attrs["device_type"] = hello.device_type
            
            diode.attrs["shot_num_init"] = settings.shot_num
            diode.attrs["trace_xvals"] = settings.trace_xvals
            diode.attrs["trace_nx"] = settings.trace_nx
            
            trace = diode.create_group("Trace")
            trace.create_dataset("shot_num", (nt_max, settings.trace_nx), dtype=np.uint16, chunks=True) # TODO: Smarter chunking
            trace.create_dataset("yvals", (nt_max, settings.trace_nx), dtype=np.uint16, chunks=True) # TODO: Smarter chunking
    
            metrics = diode.create_group("Metrics")
            metrics.create_dataset("shot_num", (nm_max,), dtype=np.uint64)
            metrics.create_dataset("trace_sum", (nm_max,), dtype=np.uint64)
            metrics.create_dataset("trace_mean", (nm_max,), dtype=np.float64)
            metrics.create_dataset("trace_max", (nm_max,), dtype=np.uint64)
            metrics.create_dataset("trace_min", (nm_max,), dtype=np.float64)
            metrics.create_dataset("trace_custom", (nm_max,), dtype=np.uint64)
      
            print(list(diode.keys()))
            #print(d1.getshot(1))
            
            i = 0 # Trace index
            j = 0 # Metrics index
            for k in range(10):
                # Client side
                data = diode_pb2.Data()
                data.trace.shot_num = 5
                data.trace.yvals = d1.getshot(i).tobytes()
                
                data.metrics.shot_num[:] = np.arange(25)
                data.metrics.trace_sum[:] = np.arange(25)*10
                data.metrics.trace_mean[:] = np.arange(25)*10
                data.metrics.trace_max[:] = np.arange(25)*10
                data.metrics.trace_min[:] = np.arange(25)*10
                data.metrics.trace_custom[:] = np.arange(25)*10
                
                # To be more general, I should consider allowing multiple traces! Then I could pack them in bunches... but probably fine for now.
                
                # Server side
                if data.HasField("trace"):
                    trace["shot_num"][i] = data.trace.shot_num
                    trace["yvals"][i, :] = np.frombuffer(data.trace.yvals, dtype=np.uint16)
                    i += 1
                
                if data.HasField("metrics"):
                    nm = len(data.metrics.shot_num)
                    print(nm)
                    
                    if j + nm > nm_max:
                        raise Warning("Metrics don't all fit in the HDF5 file! Dumping this entire batch of metrics.")
                        # TODO: Expand the HDF5 file to fit them, and then pinch it off (??)
                    else:
                        metrics["shot_num"][j:j + nm] = data.metrics.shot_num
                        metrics["trace_sum"][j:j + nm] = data.metrics.trace_sum
                        metrics["trace_mean"][j:j + nm] = data.metrics.trace_mean
                        metrics["trace_max"][j:j + nm] = data.metrics.trace_max
                        metrics["trace_min"][j:j + nm] = data.metrics.trace_min
                        metrics["trace_custom"][j:j + nm] = data.metrics.trace_custom
                    
                    j += nm
                    
                if (i > nt_max - 1) or (j > nm_max - nm - 1): # If we couldn't accomodate another batch of metrics, or couldn't accomodate another full trace
                    raise Exception("Time to pinch it off!")
                    # Ideally, we now pinch it off and create a new one with the same settings
                
                #del data
                