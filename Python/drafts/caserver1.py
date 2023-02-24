#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
caserver1.py: Test of channel access server in Python

This code is duplicated and modified directly from the pcaspy "dummy.py" dummy server code
https://raw.githubusercontent.com/paulscherrerinstitute/pcaspy/master/example/dummy.py
https://pcaspy.readthedocs.io/en/latest/tutorial.html#dummy-server

Created by Scott Feister on Jan 18 2023
"""

from pcaspy import Driver, SimpleServer
import random

prefix = 'MTEST:'

pvdb = {
    'RAND' : {
        'prec' : 3,
        'scan' : 0.5, # Update automatically once per second, not just when read
    },
}

class myDriver(Driver):
    def __init__(self):
        super(myDriver, self).__init__()

    def read(self, reason):
        if reason == 'RAND':
            value = random.random()
        else:
            value = self.getParam(reason)
        return value
        
if __name__ == '__main__':
    server = SimpleServer()
    server.createPV(prefix, pvdb)
    driver = myDriver()

    # process CA transactions
    while True:
        server.process(0.1)
