#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
diode1-client.py: Synthetic diode #1

Created by Scott Feister on Thu Jan 21 20:04:13 2021
"""

import sys
sys.path.append("../../proto")

import time
import socket
import numpy as np
from compiled_python import hello_pb2, diode_pb2

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

def recvmsg(conn):
    """ Receive a protobuf message msg, preceded by a 4-bit specification of the size
    
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
    msg = conn.recv(msgsize)
        # TODO: Error handling for socket closed
        # raise Exception("Socket closed.")
    return msg

if __name__ == "__main__":
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        with conn:
            print('Connected by', addr)
            start = time.time()
            
            # TODO: Handle breaking out of this process where needed, if recv returns zero
            msg = recvmsg(conn)
            hello = hello_pb2.Hello()
            hello.ParseFromString(msg)
            print(hello)

            msg = recvmsg(conn)
            settings = diode_pb2.Settings()
            settings.ParseFromString(msg)
            print(settings)
                    
            while True:
                msg = recvmsg(conn)
                if not msg:
                    break
                data = diode_pb2.Data()
                data.ParseFromString(msg)
                if data.trace.shot_num % 10 == 0:
                    print(data.trace.shot_num)
        end = time.time()
        print(end - start)
                