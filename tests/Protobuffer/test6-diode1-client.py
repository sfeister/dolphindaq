#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
diode1-client.py: Synthetic diode #1

Created by Scott Feister on Thu Jan 21 20:04:13 2021
"""

import sys
sys.path.append("../../proto")

from synthetic import Diode
import time
import socket
import numpy as np
from compiled_python import hello_pb2, diode_pb2

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 65432        # The port used by the server

# TODO: Error handling for unavailable socket??
def sendmsg(msg, s):
    """ Send a protobuf message msg, preceded by a 4-bit specification of the size 
    
    Inputs:
        msg     Protobuf message object
        s       Socket, connected to destination via TCP
    """
    
    msgsize_b = np.int32(msg.ByteSize()) # 4 byte message specifying length
    s.sendall(msgsize_b)
    s.sendall(msg.SerializeToString())

if __name__ == "__main__":
    xmin = 1.0e5
    xmax = 1.0e10
    nx = 800

    d1 = Diode(nx=nx)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))

        hello = hello_pb2.Hello()
        hello.unique_name = "Diode1"
        hello.unique_id = 500
        hello.device_type = hello_pb2.Hello.DIODE

        sendmsg(hello, s)

        settings = diode_pb2.Settings()
        settings.shot_num = 0
        settings.trace_xvals[:] = np.linspace(xmin, xmax, nx)

        sendmsg(settings, s)

        for i in range(1000):
            data = diode_pb2.Data()
            data.trace.shot_num = i
            data.trace.yvals = d1.getshot(i).tobytes()
            
            sendmsg(data, s)
            
            del data
            
            time.sleep(0.001)
            
        del hello
        del settings
        
        