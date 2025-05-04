#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
tft-data-test.py: A quick test of whether I can read the protos from tft.

Created by Scott Feister on May 3, 2025.
"""

import sys
sys.path.append("../../proto")

import os
import time
import numpy as np

import serial

from datetime import datetime
from compiled_python import tft_pb2 # from my custom protobufs
   
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
                if line.endswith("shot alert\r\n".encode('utf-8')):
                    msg = recvmsg(ser)
                    shot_alert = tft_pb2.ShotAlert()
                    shot_alert.ParseFromString(msg)
                    print(shot_alert)
                    image = tft_pb2.ImageWaveshare()
                    image.shot_num = shot_alert.shot_num;
                    image.nx = 280
                    image.ny = 240
                    xgv = np.linspace(0, 1, image.nx)
                    ygv = np.linspace(0, 1, image.ny)
                    X, Y = np.meshgrid(xgv, ygv)
                    Z = np.sqrt(X**2 + Y**2)*2000
                    myvals = np.round(Z.T).astype(np.uint16)
                    #myvals = np.arange(image.nx * image.ny, dtype=np.uint16)
                    image.vals = myvals.tobytes()
                    sendmsg(image, ser)
                else:
                    print(line)