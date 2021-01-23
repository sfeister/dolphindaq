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

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

if __name__ == "__main__":
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        with conn:
            print('Connected by', addr)
            start = time.time()
            
            # TODO: Handle breaking out of this process where needed, if recv returns zero
            msgsize_b = conn.recv(4)
            msgsize = np.frombuffer(msgsize_b, np.uint32)[0] # ugly, convert from bytes to uint32
            if not msgsize_b:
                # TODO: Error handling for socket closed
                raise Exception("Socket closed.")
            msg = conn.recv(msgsize)
            if not msg:
                # TODO: Error handling for socket closed
                raise Exception("Socket closed.")
            hello = hello_pb2.Hello()
            hello.ParseFromString(msg)

            print(hello)

            msgsize_b = conn.recv(4)
            if not msgsize_b:
                # TODO: Error handling for socket closed
                raise Exception("Socket closed.")
            msgsize = np.frombuffer(msgsize_b, np.uint32)[0] # ugly, convert from bytes to uint32
            msg = conn.recv(msgsize)
            if not msg:
                # TODO: Error handling for socket closed
                raise Exception("Socket closed.")
            settings = diode_pb2.Settings()
            settings.ParseFromString(msg)
            print(settings)
                    
            while True:
                msgsize_b = conn.recv(4)
                if not msgsize_b:
                    break
                msgsize = np.frombuffer(msgsize_b, np.uint32)[0] # ugly, convert from bytes to uint32
                msg = conn.recv(msgsize)
                if not msg:
                    break
                data = diode_pb2.Data()
                data.ParseFromString(msg)
                if data.trace.shot_num % 10 == 0:
                    print(data.trace.shot_num)
        end = time.time()
        print(end - start)
                