#!/bin/bash
# DATA RECORDER

DOLPHINDAQ_ROOT=/home/debian/dolphindaq

# Use the "daq" virtual environment for Python already configured in debian home directory
export WORKON_HOME=/home/debian/.virtualenvs
source /usr/share/virtualenvwrapper/virtualenvwrapper.sh
workon daq

# Run the python script within the dolhindaq folder so it can find its dependencies
cd ${DOLPHINDAQ_ROOT}/Python/pc-info-server
python pc-pva-info-server.py --directory "/home/debian/data"
