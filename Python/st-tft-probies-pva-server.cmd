#!/bin/bash

# Load the "simprobies" Python virtual environment
export WORKON_HOME=/home/softioc/.virtualenvs
source /usr/share/virtualenvwrapper/virtualenvwrapper.sh
workon simprobies

# Start the IOC
python ./tft-probies-pva-server.py --prefix "TFTPROBIES-SIM" --tty "/dev/ttyUSB-teensy1.3-02"