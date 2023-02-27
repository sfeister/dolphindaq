import epics
from time import sleep

print(epics.caget("ROSIE:info"))
print(epics.caget("ROSIE:trigger:count"))
print(epics.caget("ROSIE:trigger:decimation"))
print(epics.caget("ROSIE:rosie:output:enabled"))
print(epics.caget("ROSIE:rosie:output:first"))
print(epics.caget("ROSIE:rosie:output:count"))

epics.caput("ROSIE:rosie:output:enabled:set", 0)
sleep(2) # TODO: Add a check for it to be disabled
if not epics.caget("ROSIE:rosie:output:enabled"):
    print("Output disabled.")

epics.caput("ROSIE:rosie:output:enabled:set", 1)
while not epics.caget("ROSIE:rosie:output:enabled"):
    sleep(0.5)
    print("Waiting for Rosie device to be enabled.")

print("Output enabled.")
outfirst = epics.caget("ROSIE:rosie:output:first")
decimate = epics.caget("ROSIE:trigger:decimation")

print("Formula for current frame number in global trigger count, assuming device frameID=0 occurs with first trigger and increments with each additional trigger:")
print("global trigger of frame = {} * frameID + {}".format(decimate, outfirst))