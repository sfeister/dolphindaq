"""
This PVAccess server posts some helpful metrics from a PROBIES camera

Example usage:
    python probies-metrics-server.py -p "CAMPROBIES"

Relies on an already-existing device server, and passes forward more data under that name.
"""

import numpy as np
from time import sleep
import time

from p4p.nt import NTScalar, NTNDArray
from p4p.server import Server
from p4p.server.thread import SharedPV
from p4p.client.thread import Context

import argparse

def new_data_callback(image_p4p):
    """ Callback for each new line camera image received from p4p """
    global pvmean, pvmax, pvright1200
    image_arr = np.array(image_p4p)
    metrics_mean = np.mean(image_arr)
    metrics_max = np.max(image_arr)
    metrics_right1200 = np.mean(image_arr[1200:])
    pvmean.post(metrics_mean, timestamp=time.time())
    pvmax.post(metrics_max, timestamp=time.time())
    pvright1200.post(metrics_right1200, timestamp=time.time())
    
if __name__ == "__main__":
    # Argument parsing with help from: https://stackoverflow.com/questions/40710719/optional-command-line-arguments
    parser = argparse.ArgumentParser(description = "Description for my parser")
    parser.add_argument("-p", "--prefix", help = 'Specify the EPICS prefix for the already-existing EPICS device. Example: --prefix "MIGHTEX1"', required = True)

    argument = parser.parse_args()
    DEVICE_NAME = str(argument.prefix)

    ### INITIALIZE PVACCESS EPICS SERVER
    # https://mdavidsaver.github.io/p4p/server.html
    ctxt = Context('pva')
    pvmean = SharedPV(nt=NTScalar('d'), # scalar double
                  initial=0.0)      # setting initial value also open()'s
    pvmax = SharedPV(nt=NTScalar('d'), # scalar double
                  initial=0.0)      # setting initial value also open()'s
    pvright1200 = SharedPV(nt=NTScalar('d'), # scalar double
                  initial=0.0)      # setting initial value also open()'s
                  
    
    providers = {
        DEVICE_NAME + ':STATS:mean': pvmean,
        DEVICE_NAME + ':STATS:max': pvmax,
        DEVICE_NAME + ':STATS:right1200': pvright1200,
    }
    
    while True: ## run forever!
        try:
            i = 0
            print("Opening subscription to input and creating output server.")
            sub = ctxt.monitor(DEVICE_NAME + ':data', new_data_callback)
            with Server(providers=[providers]) as S:
                while True:
                    sleep(1)
        except KeyboardInterrupt:
            if sub:
                sub.close()
            raise
        except Exception as e: # REALLLLLLY need to catch the actual errors here
            print("Terrible error caught!!!")
            print(type(e))
            print(e)
            if sub:
                print("Closing subscription.")
                sub.close()
            print("Trying the loop again after ten seconds...")
            sleep(10)