exec O.$EPICS_HOST_ARCH/streamApp $0
dbLoadDatabase "O.Common/streamApp.dbd"
streamApp_registerRecordDeviceDriver


epicsEnvSet "STREAM_PROTOCOL_PATH","."

drvAsynIPPortConfigure ("rosie_io", "196.254.31.204:1064")

# Note that Rosie is hardcoded in firmware to this IP and port address

dbLoadRecords "rosie.db"

#log debug output to file
#streamSetLogfile StreamDebug.log

iocInit

#var streamDebug 1
