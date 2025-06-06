#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
tft-probies-pva-server.py: PVA server for the TargetTFTSerial Teensy device, assuming we want to simulate PROBIES signals.

Example Command Line Usage:
    python tft-probies-pva-server.py --prefix "TFTPROBIES-SIM" --tty "/dev/ttyACM1"

Created by Scott Feister on June 6, 2025.
"""

import sys
sys.path.append("/opt/dolphindaq/proto")

import serial

from datetime import datetime

import os
import time
import numpy as np
from skimage.transform import resize

import serial

from datetime import datetime
from compiled_python import tft_pb2, diode_pb2 # from my custom protobufs located in /opt/dolphindaq/proto. There be dragons here... too many layers of abstraction! I'm concerned. -Scott
from simprobies import infer_probies # note that "simprobies.py", custom file, this must be in the same directory as this file from which we run

from p4p.nt import NTScalar, NTNDArray
from p4p.server import Server
from p4p.server.thread import SharedPV
from p4p.client.thread import Context
from ScottNTNDArray import ScottNTNDArray # a slight modification on NTNDArray that allows the uniqueId and dataTimeStamp keywords

import argparse

# Argument parsing with help from: https://stackoverflow.com/questions/40710719/optional-command-line-arguments
parser = argparse.ArgumentParser(description = "Parser of the Teensy Serial device high-data-rate IOC")
parser.add_argument("-p", "--prefix", help = 'Specify the EPICS prefix we will give to this device. Example: --prefix "TFTPROBIES-SIM"', required = True)
parser.add_argument("-t", "--tty", help = 'Specify the TTY device port to which we will connect. Example: --tty "/dev/ttyACM1"', required = True)

argument = parser.parse_args()
DEVICE_NAME = str(argument.prefix)
SERIAL_PORT = str(argument.tty)

def print_preamble():
    print('///////////////////////////////////////')
    print('/// TFT PROBIES SIMULATOR Device Start ///')
    print('///////////////////////////////////////\n')

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

def handle_data(data, settings=None, ser=None):
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

        image = tft_pb2.ImageILI()
        image.shot_num = data.trace.shot_num;
        image.nx = 320
        image.ny = 240
        
        yvals = np.frombuffer(data.trace.yvals, dtype=np.uint16)
        print("Shot number: {}, Mean Y value: {}".format(data.trace.shot_num, np.mean(yvals)))

        # PROBIES    
        probies_metric = np.clip(np.mean(yvals) / 200, 0.0001, 0.9999)
        #print(f"PROBIES metric: {probies_metric}")
        Z = infer_probies(probies_metric)
        Z = resize(Z, [240, 240])
        Z = np.pad(Z, [(0, 0), (40, 40)])
        Z = Z / 10 * (2.0**6 - 1) # converted to six-bit, with lowered dynamic range  # NOTE: Check here for ASSUMPTIONS of dynamic range!
        #print(f"Max of z: {np.max(Z)}")
        
        # Output
        myvals = np.round(Z.T).astype(np.uint16)
        myvals = np.left_shift(myvals, 5) # Shift bits into the green channel
        #myvals = np.arange(image.nx * image.ny, dtype=np.uint16)
        image.vals = myvals.tobytes()
        if ser:
            sendmsg(image, ser)

        dev_trace.post(
            yvals,
            uniqueId=data.trace.shot_num,
            dataTimeStamp=now,
            timestamp=now,
            attrib=attrib)

    else:
        print("Data received but missing the 'Trace' field")


if __name__ == "__main__":
    print_preamble()

    ### INITIALIZE PVACCESS EPICS SERVER
    # https://mdavidsaver.github.io/p4p/server.html
    ctxt = Context('pva')
    dev_info = SharedPV(nt=NTScalar('s'), # scalar string
                  initial="TFT PROBIES PVA Server, 20250606")      # setting initial value also open()'s
    dev_trace = SharedPV(nt=ScottNTNDArray(),
                  initial=np.zeros([8]).astype(np.uint16)) # TODO: Create more options

    providers = {
        DEVICE_NAME + ':info': dev_info,
        DEVICE_NAME + ':trace': dev_trace,
    }

    settings = None
    
    with Server(providers=[providers]) as S:
        with serial.Serial(SERIAL_PORT, timeout=1) as ser:
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
                        handle_data(data, settings=settings, ser=ser) # call probies model and send back the data
                        #print(data.trace.shot_num)
                        #print(data)
                        #print("Better yvals:")
                        #print(np.frombuffer(data.trace.yvals, dtype=np.uint16))
                    else:
                        print(line)