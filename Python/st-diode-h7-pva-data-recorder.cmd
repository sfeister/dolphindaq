#!/bin/bash
# DATA RECORDER

DOLPHINDAQ_ROOT=/home/debian/dolphindaq

# Use the "daq" virtual environment for Python already configured in debian home directory
export WORKON_HOME=/home/debian/.virtualenvs
source /usr/share/virtualenvwrapper/virtualenvwrapper.sh
workon daq

# Run the python script within the dolhindaq folder so it can find its dependencies
cd ${DOLPHINDAQ_ROOT}/Python
python diode-h7-pva-data-recorder.py -p "DIODE-H7-00" -n "196.254.31.200" -o "/home/debian/data"
