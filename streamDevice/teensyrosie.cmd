exec O.$EPICS_HOST_ARCH/streamApp $0
dbLoadDatabase "O.Common/streamApp.dbd"
streamApp_registerRecordDeviceDriver

epicsEnvSet "STREAM_PROTOCOL_PATH","."

drvAsynSerialPortConfigure("teensyrosie_ino","/dev/ttyUSB-teensy1.2")
asynSetOption("teensyrosie_ino",0,"baud","115200")
asynSetOption("teensyrosie_ino",0,"bits","8")
asynSetOption("teensyrosie_ino",0,"parity","none")
asynSetOption("teensyrosie_ino",0,"stop","1")
asynSetOption("teensyrosie_ino",0,"clocal","Y")
asynSetOption("teensyrosie_ino",0,"crtscts","N")

#drvAsynIPPortConfigure "LO","localhost:40000"
#vxi11Configure "LO","192.168.1.236",0,0.0,"gpin0"

epicsThreadSleep(5) # empirically necessary to pause here, to avoid communication timeout on first SCPI commands to Arduinos. -SKF April 5 2022

dbLoadRecords "teensyrosie.db"

#log debug output to file
#streamSetLogfile StreamDebug.log

iocInit

#var streamDebug 1
