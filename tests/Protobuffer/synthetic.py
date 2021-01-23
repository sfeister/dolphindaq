#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
synthetic.py: Module for synthetic diagnostics

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
        self.data = shotnum + np.sin(shotnum) * np.sin(X)
    
    def getshot(self, shotnum):
        return self.data[shotnum]

if __name__ == "__main__":
    import matplotlib.pyplot as plt
    
    d1 = Diode()
    print(d1.getshot(100).shape)
    #for shotnum in range(nshots):
    
    fig = plt.figure(1)
    ax = fig.add_subplot(111)
    ax.plot(d1.xvals, d1.getshot(101))
    ax.plot(d1.xvals, d1.getshot(102))
    ax.plot(d1.xvals, d1.getshot(103))
