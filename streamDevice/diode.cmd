# exec O.$EPICS_HOST_ARCH/streamApp $0

dbLoadDatabase "O.Common/streamApp.dbd"
streamApp_registerRecordDeviceDriver

epicsEnvSet "STREAM_PROTOCOL_PATH","."

drvAsynIPPortConfigure ("io1", "196.254.31.200:1064")

dbLoadRecords "diode.db", "P=DIODE-00,BUS=io1"

iocInit

# Enable StreamDevice Debugging output (1) or disable output (0)
var streamDebug 0
