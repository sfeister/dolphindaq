exec O.$EPICS_HOST_ARCH/streamApp $0
dbLoadDatabase "O.Common/streamApp.dbd"
streamApp_registerRecordDeviceDriver

epicsEnvSet "STREAM_PROTOCOL_PATH","."

drvAsynSerialPortConfigure("teensycounter_ino","/dev/ttyUSB-teensy1.1")
asynSetOption("teensycounter_ino",0,"baud","115200")
asynSetOption("teensycounter_ino",0,"bits","8")
asynSetOption("teensycounter_ino",0,"parity","none")
asynSetOption("teensycounter_ino",0,"stop","1")
asynSetOption("teensycounter_ino",0,"clocal","Y")
asynSetOption("teensycounter_ino",0,"crtscts","N")

epicsThreadSleep(5) # empirically necessary to pause here, to avoid communication timeout on first SCPI commands to Arduinos. -SKF April 5 2022

dbLoadRecords "teensycounter.db"

iocInit