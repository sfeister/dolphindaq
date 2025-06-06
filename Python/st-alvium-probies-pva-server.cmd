# Load the transport layers for Alvium Vimba X
source /etc/profile.d/VimbaX_GenTL_Path_64bit.sh
source /etc/profile.d/VimbaGigETL_64bit.sh

# Load the "vimba" Python virtual environment
export WORKON_HOME=/home/softioc/.virtualenvs
source /usr/share/virtualenvwrapper/virtualenvwrapper.sh
workon vimba

# Start the IOC
python ./alvium-probies-pva-server.py --prefix "CAMPROBIES" --camera "DEV_000F3100019A"