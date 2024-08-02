#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
diode-teensy-serial-pva.py: PVA server for the TEENSYDIODE SERIAL device.

Example Command Line Usage:
    python diode-teensy-serial-pva-server.py --prefix "DIODE-01" --tty "/dev/ttyACM1"

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

from p4p.nt import NTScalar, NTNDArray
from p4p.server import Server
from p4p.server.thread import SharedPV
from p4p.client.thread import Context
from ScottNTNDArray import ScottNTNDArray # a slight modification on NTNDArray that allows the uniqueId and dataTimeStamp keywords

import argparse

# Argument parsing with help from: https://stackoverflow.com/questions/40710719/optional-command-line-arguments
parser = argparse.ArgumentParser(description = "Parser of the Teensy Serial device high-data-rate IOC")
parser.add_argument("-p", "--prefix", help = 'Specify the EPICS prefix we will give to this device. Example: --prefix "DIODE-01"', required = True)
parser.add_argument("-t", "--tty", help = 'Specify the TTY device port to which we will connect. Example: --tty "/dev/ttyACM1"', required = True)

argument = parser.parse_args()
DEVICE_NAME = str(argument.prefix)
SERIAL_PORT = str(argument.tty)

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

def pva_handle_data(data, settings=None):
    global dev_trace # PVA
    now = time.time()
    if data.HasField("trace"):
        attrib = {"shot_num" : data.trace.shot_num} # attributes
        if settings is not None:
            attrib.update({
                "start_shot_num" : settings.start_shot_num,
                "trace_nt" : settings.trace_nt,
                "dt" : settings.dt,
                "trace_dt" : settings.trace_dt,
                "trace_ymin" : settings.trace_ymin,
                "trace_ymax" : settings.trace_ymax})

        dev_trace.post(
            np.frombuffer(data.trace.yvals, dtype=np.uint16),
            uniqueId=data.trace.shot_num,
            dataTimeStamp=now,
            timestamp=now,
            attrib=attrib)

if __name__ == "__main__":

    ### INITIALIZE PVACCESS EPICS SERVER
    # https://mdavidsaver.github.io/p4p/server.html
    ctxt = Context('pva')
    dev_info = SharedPV(nt=NTScalar('s'), # scalar string
                  initial="Diode Teensy-Serial PVA Server, 20240724")      # setting initial value also open()'s
    dev_trace = SharedPV(nt=ScottNTNDArray(),
                  initial=np.zeros(8).astype("uint16")) # TODO: Create more options

    providers = {
        DEVICE_NAME + ':info': dev_info,
        DEVICE_NAME + ':trace': dev_trace,
    }

    settings = None
    
    with Server(providers=[providers]) as S:
        while True: ## run forever!
            try:
                with serial.Serial(SERIAL_PORT) as ser:
                    ser.write(b'A') # any serial message will initiate a new settings protobuf to be sent from the DiodeTeensySerial Teensy firmware
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
                                pva_handle_data(data, settings=settings) # Do anything pva-style with this data in EPICS
                                #print(data.trace.shot_num)
                                #print(data)
                                #print("Better yvals:")
                                #print(np.frombuffer(data.trace.yvals, dtype=np.uint16))
                            else:
                                pass
                                #print(line)
            except Exception as err:
                print(datetime.now())
                print(err)
                
            time.sleep(10)