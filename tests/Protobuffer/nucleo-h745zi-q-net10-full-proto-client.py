#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
nucleo-h745zi-q-net9-diode-proto-client: Client test for Nucleo H7 TCP Protobuffer Demo

Created by Scott Feister Jun 9, 2022
"""

import sys
sys.path.append("../../proto")

import time
import socket
import numpy as np
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

if __name__ == "__main__":
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))

        # Receive Hello message
        msg = recvmsg(s)
        hello = hello_pb2.Hello()
        hello.ParseFromString(msg)
        print(hello)

        # Receive Settings message
        msg2 = recvmsg(s)
        settings = diode_pb2.Settings()
        settings.ParseFromString(msg2)
        print(settings)

        # Now, receive the diode Data message in a loop
        i = 0;
        while True:
            msg3 = recvmsg(s)
            data = diode_pb2.Data()
            data.ParseFromString(msg3)
            # print(data)
            
            if i % 20 < 1:
                if data.HasField("trace") and data.trace.HasField("yvals"):
                    print(np.frombuffer(data.trace.yvals, dtype=np.uint16))
            
            print(i)
            i+=1
        