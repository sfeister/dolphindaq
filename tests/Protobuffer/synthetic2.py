#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
synthetic2.py: Module for synthetic diagnostics

v2 Uses 16-bit numbers for synthetic data

Loops endlessly

Created by Scott Feister on Thu Jan 21 20:04:13 2021
"""

import numpy as np

class Diode():
    def __init__(self, nshots=1000, nx=800, xmin=0, xmax=100):
        self.nshots = nshots
        self.nx = nx
        self.xvals = np.linspace(xmin, xmax, nx)
        shotnum = np.tile(np.arange(nshots), (nx, 1)).T # shot number array
        X = np.tile(self.xvals, (nshots, 1))
        #self.yvals = (((np.sin(shotnum) * np.sin(X) / 2.0) + 0.5) * (2**16 - 1)).astype(np.uint16)
        self.yvals = (shotnum * X).astype(np.uint16)
    
    def getshot(self, shotnum):
        return self.yvals[shotnum % self.nshots]

if __name__ == "__main__":
    import matplotlib.pyplot as plt
    
    d1 = Diode()
    print(d1.getshot(100).shape)
    #for shotnum in range(nshots):
    
    fig = plt.figure(1)
    ax = fig.add_subplot(111)
    ax.plot(d1.yvals, d1.getshot(101))
    ax.plot(d1.yvals, d1.getshot(102))
    ax.plot(d1.yvals, d1.getshot(103))
