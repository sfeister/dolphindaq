#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
nucleo-h745zi-q-net15-allstar-client: Client test for Nucleo H7 TCP Protobuffer Demo

Created by Scott Feister July 6, 2022
"""

import sys
sys.path.append("../../proto")

import time
import socket
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from compiled_python import hello_pb2, diode_pb2

HOST = '192.168.203.151'  # The server's hostname or IP address
PORT = 1234        # The port used by the server

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
    msg = conn.recv(msgsize, socket.MSG_WAITALL)
    # print(msg)
        # TODO: Error handling for socket closed
        # raise Exception("Socket closed.")
    return msg

def init_plot(settings):
    """ Create a custom plot from photodiode settings"""
    global y1_vals
    
    if settings.HasField("trace_nt"):
        trace_nt = settings.trace_nt
    else:
        trace_nt = 100
        
    xvals = np.arange(trace_nt)
    y1_vals = np.zeros_like(xvals)
        
    fig, ax = plt.subplots(figsize=(13,6))

    # create a variable for the line plot so we can later update it
    line1, = ax.plot(xvals, y1_vals, '-o', alpha=0.8)        
    
    ax.set_xlabel('Time (seconds)')
    ax.set_ylabel('Signal (Volts)')
    
    return fig, ax, line1



def animate(frame):
    """ Update the plot with current photodiode y values """
    global settings
    
    line1.set_ydata(y1_vals)

    # adjust limits if new data goes beyond bounds
    if (np.min(y1_vals) <= ax.get_ylim()[0]) or (np.max(y1_vals) >= ax.get_ylim()[1]):
        ax.set_ylim([np.min(y1_vals) - np.std(y1_vals), np.max(y1_vals) + np.std(y1_vals)])

    ax.set_title('Photodiode Data')

if __name__ == "__main__":   
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        #s.settimeout(4.0) # timeout is in seconds
        s.connect((HOST, PORT))

        # Receive Hello message
        msg = recvmsg(s)
        if msg is None:
            raise Exception("Socket closed.")
        hello = hello_pb2.Hello()
        hello.ParseFromString(msg)
        print(hello)

        # Receive Settings message
        msg2 = recvmsg(s)
        if msg2 is None:
            raise Exception("Socket closed.")

        settings = diode_pb2.Settings()
        settings.ParseFromString(msg2)
        print(settings)

        #plt.ion()
        #fig, ax, line1 = init_plot(settings)
        #ani = FuncAnimation(fig, animate, interval=500)
        
        # Now, receive the diode Data message in a loop
        i = 0;
        while True:
            msg3 = recvmsg(s)
            if msg3 is None:
                raise Exception("Socket closed.")
            data = diode_pb2.Data()
            data.ParseFromString(msg3)
            # print(data)
            
            if data.HasField("trace") and data.trace.HasField("yvals"):
                y1_vals = np.frombuffer(data.trace.yvals, dtype=np.uint16)
                print("{}, {} : {}".format(i, data.trace.shot_num, y1_vals[5]))
            
            # plt.pause(0.1)
            
            i+=1
        