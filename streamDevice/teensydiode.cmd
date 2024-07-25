exec O.$EPICS_HOST_ARCH/streamApp $0

dbLoadDatabase "O.Common/streamApp.dbd"
streamApp_registerRecordDeviceDriver

epicsEnvSet "STREAM_PROTOCOL_PATH","."

drvAsynSerialPortConfigure("io1","/dev/ttyACM2")
asynSetOption("io1",0,"baud","115200")
asynSetOption("io1",0,"bits","8")
asynSetOption("io1",0,"parity","none")
asynSetOption("io1",0,"stop","1")
asynSetOption("io1",0,"clocal","Y")
asynSetOption("io1",0,"crtscts","N")

dbLoadRecords "teensydiode.db", "P=TEENSYDIODE,BUS=io1"

iocInit

# Enable StreamDevice Debugging output (1) or disable output (0)
var streamDebug 0