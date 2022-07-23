#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
hamaline-h7-data-recorder.py: A simple data recorder for the HAMALINE-H7 line camera device.

Spun off from diode-h7-data-recorder.py on July 22, 2022.

Created by Scott Feister on July 22, 2022.
"""

import sys
sys.path.append("../proto")

import os
import time
import socket
import numpy as np
import h5py

from datetime import datetime
from flsuite import sftools as sf
from compiled_python import hello_pb2, linecam_pb2

try:
    import Adafruit_BBIO.GPIO as GPIO  # for flashing LED indicator on Beaglebone Black
    bbio = True
except:
    bbio = False
    
DATADIR = r"C:\Users\scott\Documents\temp\july2022\data" # this folder should already exist on this computer, and is where data will be stored
HOST = '192.168.203.151'  # The remote device's hostname or IP address
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

def init_linecam_h5(f, hello, settings, ni_max, nm_max):
    """ Initialize the LineCam HDF5 file 
    Inputs:
        f: Open, empty HDF5 file
        hello: Protobuf object from hello message
        settings: Protobuf object from LineCam settings message
    Outputs:
        IO on linecam.hdf5
        linecam: HDF5 handle to the LineCam category
        image: HDF5 handle to the Image category (sub-category of LineCam)
        metrics: HDF5 handle to the Metrics category (sub-category of LineCam)
    """
    
    linecam = f.create_group(hello.unique_name)
    
    # TODO: Add versioning of protobufs            
    linecam.attrs["unique_name"] = hello.unique_name
    linecam.attrs["unique_id"] = hello.unique_id
    linecam.attrs["device_type"] = hello.device_type
    
    linecam.attrs["start_shot_num"] = settings.start_shot_num
    linecam.attrs["image_nx"] = settings.image_nx
    # TODO: Add other stuff here
    
    image = linecam.create_group("Image")
    image.create_dataset("shot_num", (ni_max,), maxshape=(ni_max,), dtype=np.uint64, chunks=True) # TODO: Smarter chunking
    image.create_dataset("yvals", (ni_max, settings.image_nx), maxshape=(ni_max, settings.image_nx), dtype=np.uint16, chunks=True) # TODO: Smarter chunking
    
    metrics = linecam.create_group("Metrics")
    metrics.create_dataset("shot_num", (nm_max,), dtype=np.uint64, chunks=True)
    metrics.create_dataset("shot_time_seconds", (nm_max,), dtype=np.int64, chunks=True)
    metrics.create_dataset("shot_time_nanos", (nm_max,), dtype=np.int32, chunks=True)
    metrics.create_dataset("shot_time_alt_nanos", (nm_max,), dtype=np.uint64, chunks=True)
    metrics.create_dataset("image_mean", (nm_max,), dtype=np.double, chunks=True)
    metrics.create_dataset("image_max", (nm_max,), dtype=np.uint64, chunks=True)
    
    return linecam, image, metrics
                
def insert_image_h5(image_h5, image_pbuf, i):
    """ Insert a protobuf image message into the HDF5 file at index i """
    # TODO: Check that it will fit (?) Or perhaps just handle the error message
    image_h5["shot_num"][i] = image_pbuf.shot_num
    image_h5["yvals"][i, :] = np.frombuffer(image_pbuf.yvals, dtype=np.uint16)

def insert_metrics_h5(metrics_h5, metrics_pbuf, j):
    """ Insert a set of protobuf linecam metrics message into the HDF5 file at index j """
    nm = len(metrics_pbuf.shot_num)
    
    # TODO: Check that it will fit (?) Or perhaps just handle the error message
    metrics_h5["shot_num"][j:j + nm] = metrics_pbuf.shot_num
    metrics_h5["shot_time_seconds"][j:j + nm] = metrics_pbuf.shot_time_seconds
    metrics_h5["shot_time_nanos"][j:j + nm] = metrics_pbuf.shot_time_nanos
    metrics_h5["shot_time_alt_nanos"][j:j + nm] = metrics_pbuf.shot_time_alt_nanos
    metrics_h5["image_mean"][j:j + nm] = metrics_pbuf.image_mean
    metrics_h5["image_max"][j:j + nm] = metrics_pbuf.image_max

def resize_image_h5(image_h5, nshots):
    """ Resize the HDF5 arrays related to linecam images, truncating or expanding to "nshots" shots """
    for grpname in ["shot_num", "yvals"]:
        image_h5[grpname].resize(nshots, axis=0)

def resize_metrics_h5(metrics_h5, nshots):
    """ Resize the HDF5 arrays related to linecam metrics, truncating or expanding to "nshots" shots """
    for grpname in ["shot_num", "shot_time_seconds", "shot_time_nanos", "shot_time_alt_nanos", "image_mean", "image_max"]:
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
    if hello.device_type == hello_pb2.Hello.LINECAM:
        msg = recvmsg(conn)
        if not msg:
           print("No linecam message.")
           return # HDF5 file isn't yet open. Break out of function immediately if TCP connection closed here.
        settings = linecam_pb2.Settings()
        settings.ParseFromString(msg)
        print(settings)
                
        ni_max = 30000 # Max number of shots with images to store in a single HDF5 file (About 4 kB per image. Image settings on device will set the file size.) 
        nm_max = 30000 # Max number of shots with metrics to store in a single HDF5 file (60k = 60 seconds. Metrics settings here will set the batch time duration.)
        
        while msg:
            start = time.time()
            # Go endlessly in the following loop, until TCP connection is closed.
            now = datetime.now() # current date and time
            outdir = sf.subdir2(DATADIR, now.strftime("%Y-%m-%d")) # Directory of today's date
            h5fn = os.path.join(outdir, hello.unique_name + "-" + now.strftime("%Y-%m-%d_%Hh%Mm%Ss%f")[:-3] + "ms.h5") # HDF5 filename
            print("Opening file: " + h5fn)
            with h5py.File(h5fn, 'w') as f: # Need a new one every time settings change
                ftimespec = r"%Y-%m-%d %H:%M:%S.%f"
                f.attrs["Time Format"] = ftimespec
                f.attrs["File Creation Time"] = now.strftime(ftimespec)
                f.attrs["Original Filename"] = h5fn
                # TODO: Put this analysis script in the file as well? IDK.
    
                linecam_h5, image_h5, metrics_h5 = init_linecam_h5(f, hello, settings, ni_max, nm_max)
            
                i = 0 # Trace index
                j = 0 # Metrics index
                nm = 0 # Number of metrics per packet
                
                while (i < ni_max) and (j < nm_max - nm):
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
                    data = linecam_pb2.Data()
                    data.ParseFromString(msg)
    
                    # Store the data message appropriately
                    if data.HasField("image"):
                        if data.image.shot_num % 10 == 0:   # DEBUG ONLY
                            print(data.image.shot_num)      # DEBUG ONLY

                        insert_image_h5(image_h5, data.image, i)
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
                
                f.attrs["File Closure Time"] = datetime.now().strftime(ftimespec)
                resize_image_h5(image_h5, i)
                resize_metrics_h5(metrics_h5, j)
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

                