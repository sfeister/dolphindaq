#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
diode-teensy-serial.py: A data recorder for the TEENSYDIODE SERIAL device.

Forked by Scott Feister on July 24, 2024.
"""

import sys
sys.path.append("../proto")

import os
import time
import numpy as np

import serial

from datetime import datetime
from compiled_python import hello_pb2, diode_pb2
   
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

if __name__ == "__main__":
    with serial.Serial('/dev/ttyACM1', timeout=1) as ser:
        while True:
            line = ser.readline()
            if line:
                if line.endswith("settings\r\n".encode('utf-8')):
                    msg = recvmsg(ser)
                    settings = diode_pb2.Settings()
                    settings.ParseFromString(msg)
                    print(settings)
                elif line.endswith("data\r\n".encode('utf-8')):
                    msg = recvmsg(ser)
                    data = diode_pb2.Data()
                    data.ParseFromString(msg)
                    print(data)
                    print("Better yvals:")
                    print(np.frombuffer(data.trace.yvals, dtype=np.uint16))
                else:
                    print(line)

