# streamDevice

Each Beaglebone Black Rev3 has at least one Nucleo board attached via USB. The Beaglebone sends commands and receives data from the Nucleo via USB.

USB serial communication is translated to and from EPICS process variables via an EPICS module called [StreamDevice](
http://epics.web.psi.ch/software/streamdevice/).

"streamDevice" settings files for each EPICS input/output controller (on the Raspberry Pi) are contained in this folder.