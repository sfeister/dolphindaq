import sys
sys.path.append("../../proto")

import time
import socket
import numpy as np
import matplotlib.pyplot as plt
# from compiled_python import hello_pb2, diode_pb2 # ModuleNotFoundError: No module named 'compiled_python'

HOST = '192.168.203.151'  # The server's hostname or IP address
PORT = 1234        # The port used by the server

xdata = np.linspace(0,10,100)

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

def live_plotter(x_vec,y1_data,line1,identifier='',pause_time=0.1):
    if line1==[]:
        # this is the call to matplotlib that allows dynamic plotting
        plt.ion()
        ax = fig.add_subplot(111)
        # create a variable for the line so we can later update it
        line1, = ax.plot(x_vec,y1_data,'-o',alpha=0.8)        
        #update plot label/title
        plt.ylabel('Y Label')
        plt.title('Title: {}'.format(identifier))
        plt.show()
    # after the figure, axis, and line are created, we only need to update the y-data
    line1.set_ydata(y1_data)
    # adjust limits if new data goes beyond bounds
    if np.min(y1_data)<=line1.axes.get_ylim()[0] or np.max(y1_data)>=line1.axes.get_ylim()[1]:
        plt.ylim([np.min(y1_data)-np.std(y1_data),np.max(y1_data)+np.std(y1_data)])
    # this pauses the data so the figure/axis can catch up - the amount of pause can be altered above
    plt.pause(pause_time)
    
    # return line so we can update it again in the next iteration
    return line1

fig = plt.figure(figsize=(13,6))

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
                    live_plotter(xdata, data.trace.yvals, [], 'NUCLEO DATA', 0.5)
            
            print(i)
            i+=1

