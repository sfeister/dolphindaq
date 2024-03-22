"""
PC Info Channel Access Server

Run this on various PCs to broadcast some generic info such as the wall clock time and the amount of free disk space

EPICS CA PVs broadcast:
    $HOSTNAME:clock:unix_time     Current unix time on this PC
    $HOSTNAME:disk:free           Current free disk space on this PC (in GB)

Should work on either Linux or Windows.
May require Python 3.9+ to get the time properly.

Created by Scott Feister on March 22 2024
"""

import os
import numpy as np

import datetime
import socket # for getting our hostname
import shutil # for getting our disk free space

from softioc import softioc, builder, asyncio_dispatcher
import asyncio # Note that cothread didn't build with pip on Windows, so using asyncio
import logging

from time import sleep, perf_counter

DEVICE_NAME = socket.gethostname()

if __name__ == "__main__":
    print("Device name is: " + DEVICE_NAME)
    
    ### INITIALIZE CA EPICS SERVER
    # Create an asyncio dispatcher, the event loop is now running
    dispatcher = asyncio_dispatcher.AsyncioDispatcher()

    logging.basicConfig(level=logging.DEBUG)

    # Set the record prefix
    builder.SetDeviceName(DEVICE_NAME)
    builder.SetBlocking(True)

    ## Create some records
    dev_info = builder.stringIn("info", initial_value="PC Clock CA Server, 20240322")
    dev_clock_str = builder.stringIn("clock:time", initial_value=str(datetime.datetime.now()))
    dev_clock_unix = builder.aIn("clock:unix_time", initial_value=datetime.datetime.now().astimezone().timestamp())
    dev_free_GB = builder.longIn("disk:free", initial_value=0)
            
    # Boilerplate get the IOC started
    builder.LoadDatabase()
    softioc.iocInit(dispatcher)
    
    # Start processes required to be run after iocInit
    async def update():
        while True:
            free_GB = np.int(np.round(shutil.disk_usage('/').free / (1024 * 1024 * 1024)))
            dev_free_GB.set(free_GB)
            for i in range(20): # update wall time more frequently than other stats
                now = datetime.datetime.now()
                dev_clock_str.set(str(now))
                dev_clock_unix.set(now.astimezone().timestamp())
                await asyncio.sleep(0.25)

    dispatcher(update)

    # Finally leave the IOC running with an interactive shell.
    softioc.interactive_ioc(globals())