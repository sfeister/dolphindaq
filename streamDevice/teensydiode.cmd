exec O.$EPICS_HOST_ARCH/streamApp $0
dbLoadDatabase "O.Common/streamApp.dbd"
streamApp_registerRecordDeviceDriver


epicsEnvSet "STREAM_PROTOCOL_PATH","."

drvAsynSerialPortConfigure("teensydiode_ino","/dev/ttyUSB-teensy1.2")
asynSetOption("teensydiode_ino",0,"baud","115200")
asynSetOption("teensydiode_ino",0,"bits","8")
asynSetOption("teensydiode_ino",0,"parity","none")
asynSetOption("teensydiode_ino",0,"stop","1")
asynSetOption("teensydiode_ino",0,"clocal","Y")
asynSetOption("teensydiode_ino",0,"crtscts","N")

epicsThreadSleep(5) # May not still be needed; was empirically necessary to pause here, to avoid communication timeout on first SCPI commands to Arduinos, circa April 2022 for sidekick system

dbLoadRecords "teensydiode.db"

#log debug output to file
#streamSetLogfile StreamDebug.log

iocInit

#var streamDebug 1
