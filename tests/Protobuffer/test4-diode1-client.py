#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
diode1-client.py: Synthetic diode #1

Created by Scott Feister on Thu Jan 21 20:04:13 2021
"""

from synthetic import Diode
import time
import socket
import numpy as np
import hello_pb2
import diode_pb2

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 65432        # The port used by the server
    
if __name__ == "__main__":
    nshots = 1000
    xmin = 1.0e5
    xmax = 1.0e10
    nx = 800
    d1 = Diode(nshots = nshots, nx=nx)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))

        hello = hello_pb2.Hello()
        hello.unique_name = "Diode1"
        hello.unique_id = 500
        hello.device_type = hello_pb2.Hello.DIODE

        msg = hello
        msgsize_b = np.int32(msg.ByteSize()) # 4 byte message specifying length
        s.sendall(msgsize_b)
        s.sendall(msg.SerializeToString())
        del hello

        settings = diode_pb2.Settings()
        settings.shot_num = 0
        settings.trace_xvals[:] = np.linspace(xmin, xmax, nx)
 
        msg = settings
        msgsize_b = np.int32(msg.ByteSize()) # 4 byte message specifying length
        s.sendall(msgsize_b)
        s.sendall(msg.SerializeToString())
        del settings

        for i in range(nshots):
            data = diode_pb2.Data()
            data.trace.shot_num = i
            data.trace.yvals = d1.getshot(i).tobytes()
            
            msg = data
            msgsize_b = np.int32(msg.ByteSize()) # 4 byte message specifying length
            s.sendall(msgsize_b)
            s.sendall(msg.SerializeToString())
            
            del data
            
            time.sleep(0.01)
        
        