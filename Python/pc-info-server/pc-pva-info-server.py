"""
PC Info Channel Access Server

Run this on various PCs to broadcast some generic info such as the wall clock time and the amount of free disk space.

Example usage:
    python pc-pva-info-server.py --directory "/home/debian/data"

EPICS PVA PVs broadcast:
    $HOSTNAME:clock:time          Human-readable time on this PC
    $HOSTNAME:clock:unix_time     Current unix time on this PC
    $HOSTNAME:disk:free           Current free disk space on this PC or specified folder (in GB)

Should work on either Linux or Windows.
May require Python 3.9+ to get the time properly.

Created by Scott Feister on March 22 2024
"""

import os
import numpy as np

import datetime
import socket # for getting our hostname
import shutil # for getting our disk free space

import time
from time import sleep, perf_counter

from p4p.nt import NTScalar, NTNDArray
from p4p.server import Server
from p4p.server.thread import SharedPV
from p4p.client.thread import Context

import argparse

# Argument parsing with help from: https://stackoverflow.com/questions/40710719/optional-command-line-arguments
parser = argparse.ArgumentParser(description = "Description for my parser")
parser.add_argument("-p", "--prefix", help = 'Override the default EPICS prefix to use for this device -- by default, the PC hostname. Example: --prefix "TargetPC"', required = False, default=socket.gethostname())
parser.add_argument("-d", "--directory", help = 'Override the default directory to monitor for free space -- by default, the root directory of the PC. Example: --directory "/home/debian/data"', required = False, default="/")

argument = parser.parse_args()
DEVICE_NAME = str(argument.prefix)  # Needs to match unique EPICS device name
DISK_DIRECTORY = str(argument.directory) # The disk directory to monitor for free space

if __name__ == "__main__":
    print("Device name is: " + DEVICE_NAME)
    
    ### INITIALIZE PVACCESS EPICS SERVER
    # https://mdavidsaver.github.io/p4p/server.html
    ctxt = Context('pva')
    dev_info = SharedPV(nt=NTScalar('s'), # scalar string
                  initial="PC Info PVA Server, 20240327")      # setting initial value also open()'s
    dev_clock_str = SharedPV(nt=NTScalar('s'), # scalar string
                  initial=str(datetime.datetime.now()))      # setting initial value also open()'s
    dev_clock_unix = SharedPV(nt=NTScalar('d'), # scalar double
                  initial=datetime.datetime.now().astimezone().timestamp())      # setting initial value also open()'s
    dev_free_GB = SharedPV(nt=NTScalar('i'), # scalar int32 
                  initial=0)      # setting initial value also open()'s
    
    providers = {
        DEVICE_NAME + ':info': dev_info,
        DEVICE_NAME + ':clock:time': dev_clock_str,
        DEVICE_NAME + ':clock:unix_time': dev_clock_unix,
        DEVICE_NAME + ':disk:free': dev_free_GB
    }
    
    with Server(providers=[providers]) as S:
        while True:
            free_GB = np.int64(np.round(shutil.disk_usage(DISK_DIRECTORY).free / (1024 * 1024 * 1024)))
            dev_free_GB.post(free_GB)
            for i in range(20): # update wall time more frequently than other stats
                now = datetime.datetime.now()
                dev_clock_str.post(str(now))
                dev_clock_unix.post(now.astimezone().timestamp())
                sleep(0.25)