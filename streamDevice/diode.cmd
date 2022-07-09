exec O.$EPICS_HOST_ARCH/streamApp $0
dbLoadDatabase "O.Common/streamApp.dbd"
streamApp_registerRecordDeviceDriver


epicsEnvSet "STREAM_PROTOCOL_PATH","."

drvAsynSerialPortConfigure("diode_io","/dev/ttyUSB0")
asynSetOption("diode_io",0,"baud","115200")
asynSetOption("diode_io",0,"bits","8")
asynSetOption("diode_io",0,"parity","none")
asynSetOption("diode_io",0,"stop","1")
asynSetOption("diode_io",0,"clocal","Y")
asynSetOption("diode_io",0,"crtscts","N")

epicsThreadSleep(5) # May not still be needed; was empirically necessary to pause here, to avoid communication timeout on first SCPI commands to Arduinos, circa April 2022 for sidekick system

dbLoadRecords "diode.db"

#log debug output to file
#streamSetLogfile StreamDebug.log

iocInit

#var streamDebug 1
