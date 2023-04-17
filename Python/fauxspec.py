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

import RPi.GPIO as GPIO
import signal
import sys

from time import sleep, perf_counter

TRIGD_GPIO = 26
EXTI_GPIO = 16

global trigcnt, triggered
trigcnt = 0

def signal_handler(sig, frame):
    GPIO.cleanup()
    sys.exit(0)

def exti_callback(channel):
    # When trigger arrives, increment trigger count and request image display
    global trigcnt
    trigcnt += 1
    if not GPIO.input(TRIGD_GPIO):
        GPIO.output(TRIGD_GPIO, 1)
    

if __name__ == "__main__":
    # imgdir = r"C:\Users\scott\Documents\DATA\2023-03-24 Sample Particle Spectrometer Images\SampleImages"
    imgdir = "/home/pi/fauxspec/SampleImages"
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
        ims[i] = cv2.imread(pnglist[i], cv2.IMREAD_COLOR)
        
        # Add a label
        position = (10,50)
        cv2.putText(
             ims[i], #numpy array on which text is written
             "Img #: {}".format(i), #text
             position, #position at which writing has to start
             cv2.FONT_HERSHEY_SIMPLEX, #font family
             1, #font size
             (209, 80, 0, 255), #font color
             3) #font stroke

    background = cv2.imread(bg_filename, cv2.IMREAD_COLOR)
    
    #### Initialize GPIO interrupt (external trigger)
    
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(TRIGD_GPIO, GPIO.OUT)
    GPIO.setup(EXTI_GPIO, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.add_event_detect(EXTI_GPIO, GPIO.RISING, 
        callback=exti_callback, bouncetime=200)
    
    signal.signal(signal.SIGINT, signal_handler) # Catch Ctrl+c for cleanup
    
    trigcnt = 0
    
    
    def callback_trigger():
        t1_start = perf_counter()
        index_img = trigcnt % NUM_IMS
        cv2.imshow("espec", ims[index_img])
        retval=cv2.waitKey(50) # Flash signal for about 50 milliseconds (might take longer to process updates)
        if retval != -1: # key was pressed
            return retval
        cv2.imshow("espec", background)
        retval=cv2.waitKey(50) # Gives GUI another 50 dedicated milliseconds to process updates
        if retval != -1: # key was pressed
            return retval
        t1_stop = perf_counter()
        print("Elapsed time: {} milliseconds".format(np.round((t1_stop - t1_start)*1000)))
        return -1
    
    cv2.setWindowProperty("espec", cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
    
    while True:
        if GPIO.input(TRIGD_GPIO):
            retval = callback_trigger()
            if retval != -1: # key was pressed
                break
            GPIO.output(TRIGD_GPIO, 0)
        retval=cv2.waitKey(500)
        if retval != -1: # key was pressed
            break

    # closing all open windows
    cv2.destroyAllWindows()