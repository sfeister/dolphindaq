#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
alvium-probies-pva-server.py: PVA server for the Alvium GigE CAMPROBIES device.

Sets up camera triggering, a fixed delay, a fixed exposure time, a fixed gain, etc.

Example Command Line Usage:
    python camprobies.py --prefix "CAMPROBIES" --camera "DEV_000F3100019A"

Created by Scott Feister on June 6, 2025.
Some code copied from VimbaX Python API Example code: asynchronous_grab.py
"""

import os
import time
import numpy as np

import sys
from typing import Optional, Tuple
from vmbpy import *

from datetime import datetime

from p4p.nt import NTScalar, NTNDArray
from p4p.server import Server
from p4p.server.thread import SharedPV
from p4p.client.thread import Context
from ScottNTNDArray import ScottNTNDArray # a slight modification on NTNDArray that allows the uniqueId and dataTimeStamp keywords

import argparse

# Argument parsing with help from: https://stackoverflow.com/questions/40710719/optional-command-line-arguments
parser = argparse.ArgumentParser(description = "Parser of the Teensy Serial device high-data-rate IOC")
parser.add_argument("-p", "--prefix", help = 'Specify the EPICS prefix we will give to this device. Example: --prefix "CAMPROBIES"', required = True)
parser.add_argument("-c", "--camera", help = 'Specify the Alvium camera ID to which we will connect. Example: --camera "DEV_000F3100019A"', required = True)

argument = parser.parse_args()
DEVICE_NAME = str(argument.prefix)
CAM_ID = str(argument.camera)
TRIGGER_DELAY_US = 130000 # time delay between the trigger arriving and the camera exposure beginning, in microseconds
EXPOSURE_TIME_US = 1972.727051
GAIN = 0.0

def print_preamble():
    print('///////////////////////////////////////')
    print('/// CAMPROBIES Device Start ///')
    print('///////////////////////////////////////\n')

def abort(reason: str, return_code: int = 1, usage: bool = False):
    print(reason + '\n')

    if usage:
        print_usage()

    sys.exit(return_code)

def get_camera(camera_id: Optional[str]) -> Camera:
    with VmbSystem.get_instance() as vmb:
        if camera_id:
            try:
                return vmb.get_camera_by_id(camera_id)

            except VmbCameraError:
                abort('Failed to access Camera \'{}\'. Abort.'.format(camera_id))

        else:
            cams = vmb.get_all_cameras()
            if not cams:
                abort('No Cameras accessible. Abort.')

            return cams[0]


def setup_camera(cam: Camera):
    with cam:
        # Restore settings to initial value.
        try:
            cam.UserSetSelector.set('Default')

        except (AttributeError, VmbFeatureError):
            abort('Failed to set Feature \'UserSetSelector\'')

        try:
            cam.UserSetLoad.run()
            print("--> All feature values have been restored to default")

        except (AttributeError, VmbFeatureError):
            abort('Failed to run Feature \'UserSetLoad\'')

        cam.TriggerMode.set('On')
        cam.TriggerSource.set('Line3')
        cam.TriggerDelay.set(TRIGGER_DELAY_US) # in microseconds
        cam.ExposureAuto.set('Off')
        cam.ExposureTime.set(EXPOSURE_TIME_US) # in microseconds
        cam.GainAuto.set('Off')
        cam.Gain.set(GAIN)
        cam.Gamma.set(1.0) # default
        print("--> Features now updated for triggered acquisition.")

        # Try to adjust GeV packet size. This Feature is only available for GigE - Cameras.
        try:
            stream = cam.get_streams()[0]
            stream.GVSPAdjustPacketSize.run()

            while not stream.GVSPAdjustPacketSize.is_done():
                pass

        except (AttributeError, VmbFeatureError):
            pass


def frame_handler(cam: Camera, stream: Stream, frame: Frame):
    global dev_image # PVA
    now = time.time()
    
    imarray = frame.as_numpy_ndarray()[:,:,0]
    #imarray = np.zeros([544]).astype(np.uint8)
    
    #print("Shape: {}".format(imarray.shape))
    
    attrib = {"shot_num" : frame.get_id()}
    
    attrib.update({
        "metric_mean" : imarray.mean().item(), # note that the ".item()" converts this scalar from a Numpy type to a Python native type, which is needed to wrap properly
        "metric_max" : imarray.max().item(),
        })


    print('{} acquired {}'.format(cam, frame), flush=True)
    #attrib={}

    if frame.get_status() == FrameStatus.Complete:   
        dev_image.post(
            imarray,
            uniqueId=frame.get_id(),
            dataTimeStamp=frame.get_timestamp(),
            timestamp=now,
            attrib=attrib)

    cam.queue_frame(frame)

if __name__ == "__main__":
    print_preamble()
    cam_id = CAM_ID
    allocation_mode = AllocationMode.AnnounceFrame

    ### INITIALIZE PVACCESS EPICS SERVER
    # https://mdavidsaver.github.io/p4p/server.html
    ctxt = Context('pva')
    dev_info = SharedPV(nt=NTScalar('s'), # scalar string
                  initial="CAMPROBIES PVA Server, 20250606")      # setting initial value also open()'s
    dev_image = SharedPV(nt=ScottNTNDArray(),
                  initial=np.zeros([8,8]).astype(np.uint8)) # TODO: Create more options

    providers = {
        DEVICE_NAME + ':info': dev_info,
        DEVICE_NAME + ':image': dev_image,
    }

    settings = None
    
    with Server(providers=[providers]) as S:
        with VmbSystem.get_instance():
            with get_camera(cam_id) as cam:

                setup_camera(cam)
                print('Press <enter> to stop Frame acquisition.')

                try:
                    # Start Streaming with a custom a buffer of 10 Frames (defaults to 5)
                    cam.start_streaming(handler=frame_handler,
                                        buffer_count=10,
                                        allocation_mode=allocation_mode)
                    input()

                finally:
                    cam.stop_streaming()
