#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
fauxspec.py: A fake 2D spectrometer data source. Triggered to display image data. Cycles through a folder full of images, displaying each one after the other on an external display when a trigger arrives.

References:
1. OpenCV API: https://docs.opencv.org/4.7.0/index.html

Created by Scott Feister on March 24, 2023.
"""

import cv2
import os
import glob
import numpy as np
from time import sleep, perf_counter


if __name__ == "__main__":
    imgdir = r"C:\Users\scott\Documents\DATA\2023-03-24 Sample Particle Spectrometer Images\SampleImages"
    pnglist = glob.glob(os.path.join(imgdir, r'image*.png'))
    bg_filename = os.path.join(imgdir, "background.png")
    print(pnglist)
        
    # Fullscreen OpenCV
    # https://stackoverflow.com/questions/17696061/how-to-display-a-full-screen-images-with-python2-7-and-opencv2-4
    # https://forum.opencv.org/t/how-to-expand-image-to-fullscreen-with-imshow-method/6924/4
    cv2.namedWindow("espec", cv2.WINDOW_NORMAL)          
    cv2.setWindowProperty("espec", cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)

    # Buffer the images into RAM
    NUM_IMS = len(pnglist)
    ims = [None]*NUM_IMS
    for i in range(NUM_IMS):
        ims[i] = cv2.imread(pnglist[i], cv2.IMREAD_REDUCED_GRAYSCALE_4)

    background = cv2.imread(bg_filename, cv2.IMREAD_REDUCED_GRAYSCALE_4)
    
    trigcnt = 0
    
    def callback_trigger():
        global trigcnt
        trigcnt += 1
        index_img = trigcnt % NUM_IMS
        cv2.imshow("espec", ims[index_img])
        key=cv2.waitKey(50) # Flash signal for about 50 milliseconds (might take longer to process updates)
        cv2.imshow("espec", background)
        key=cv2.waitKey(50) # Gives GUI another 50 dedicated milliseconds to process updates

    for i in range(20):
        callback_trigger()
        sleep(0.5)
        
    # closing all open windows
    cv2.destroyAllWindows()