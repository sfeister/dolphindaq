#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
tft-data-test.py: A quick test of whether I can read the protos from tft.

Created by Scott Feister on May 3, 2025.
"""

import sys
sys.path.append("../../proto")

import serial

from datetime import datetime

import os
import time
import numpy as np

import serial

from datetime import datetime
from compiled_python import tft_pb2, diode_pb2 # from my custom protobufs

def handle_data(data, settings=None, ser=None):
    now = time.time()
    assert data.HasField("trace")
    
    attrib = {"shot_num" : data.trace.shot_num} # attributes
    if settings is not None:
        attrib.update({
            "start_shot_num" : settings.start_shot_num,
            "trace_nt" : settings.trace_nt,
            "dt" : settings.dt,
            "trace_dt" : settings.trace_dt,
            "trace_ymin" : settings.trace_ymin,
            "trace_ymax" : settings.trace_ymax})
            
    print(data.trace.yvals)
    print(data.trace.shot_num)
    
    image = tft_pb2.ImageILI()
    image.shot_num = data.trace.shot_num;
    image.nx = 320
    image.ny = 240
    xgv = np.linspace(0, 1, image.nx)
    ygv = np.linspace(0, 240./320., image.ny)
    X, Y = np.meshgrid(xgv, ygv)
    Z = np.sqrt(X**2 + Y**2)/np.sqrt(2)*(2.0**6 - 1)
    myvals = np.round(Z.T).astype(np.uint16)
    myvals = np.left_shift(myvals, 5) # Shift bits into the green channel
    #myvals = np.arange(image.nx * image.ny, dtype=np.uint16)
    image.vals = myvals.tobytes()
    if ser:
        sendmsg(image, ser)


def recvmsg(ser):
    """ Receive a protobuf message msg, preceded by a readline specification of the size
    
    Inputs:
        ser    Already-open PySerial connection
        
    Outputs:
        msg     Protobuf message object
    """
    msgsize_str = ser.readline()
    msgsize = int(msgsize_str)
    
    msg = bytes();
    while len(msg) < msgsize:
        msg_tmp = ser.read(msgsize - len(msg))
        if not msg_tmp:
            return None
        else:
            msg += msg_tmp
    return msg

def sendmsg(image, ser):
    payload = image.SerializeToString()
    payload_len = len(payload)
    print("Payload length: ", payload_len)

    payload_message = f"{payload_len}\r\n".encode('utf-8')
    ser.write(payload_message)
    print(f"Sent: {payload_message} bytes")
    ser.write(payload)
    
if __name__ == "__main__":
    with serial.Serial('COM13', timeout=1) as ser:
        while True:
            line = ser.readline()
            if line:
                if line.endswith("settings\r\n".encode('utf-8')):
                    msg = recvmsg(ser)
                    settings = diode_pb2.Settings()
                    settings.ParseFromString(msg)
                    print("Settings updated.")
                    #print(settings)
                elif line.endswith("data\r\n".encode('utf-8')):
                    msg = recvmsg(ser)
                    data = diode_pb2.Data()
                    data.ParseFromString(msg)
                    handle_data(data, settings=settings, ser=ser) # Do anything pva-style with this data in EPICS
                    #print(data.trace.shot_num)
                    #print(data)
                    #print("Better yvals:")
                    #print(np.frombuffer(data.trace.yvals, dtype=np.uint16))
                else:
                    print(line)