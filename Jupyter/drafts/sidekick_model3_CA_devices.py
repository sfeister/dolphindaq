# Ophyd Devices for the Channel Access PVs of Sidekick Model 3
# Created by Scott Feister on June 22 2026
#
# Copied from channel list at:
#     https://sidekickscience.com/model3/model3-process-variables/

import numpy as np
from ophyd import Component, Device, EpicsSignal, EpicsSignalRO

class PulseGenerator(Device):
    """ Sidekick Model 3 Teensy PulseGenerator """
    info = Component(EpicsSignalRO, 'info')
    debug = Component(EpicsSignal, 'debug')
    trigger_count = Component(EpicsSignal, 'trigger:count', write_pv='trigger:count:set')
    output_enabled = Component(EpicsSignal, 'output:enabled', write_pv='output:enabled:set')
    reprate = Component(EpicsSignal, 'reprate', write_pv='reprate:set')
    ch1_delay = Component(EpicsSignal, 'CH1:delay', write_pv='CH1:delay:set')
    ch2_delay = Component(EpicsSignal, 'CH2:delay', write_pv='CH2:delay:set')
    ch3_delay = Component(EpicsSignal, 'CH3:delay', write_pv='CH3:delay:set')
    ch4_delay = Component(EpicsSignal, 'CH4:delay', write_pv='CH4:delay:set')

class LilLaser(Device):
    """ Sidekick Model 3 Teensy LilLaser """
    info = Component(EpicsSignalRO, 'info')
    debug = Component(EpicsSignal, 'debug')
    trigger_count = Component(EpicsSignal, 'trigger:count', write_pv='trigger:count:set')
    output_enabled = Component(EpicsSignal, 'output:enabled', write_pv='output:enabled:set')
    powers_nt = Component(EpicsSignalRO, 'powers:nt')
    powers_dt = Component(EpicsSignal, 'powers:dt', write_pv='powers:dt:set')
    powers = Component(EpicsSignal, 'powers', write_pv='powers:set')

class Diode(Device):
    """ Sidekick Model 3 Teensy Diode (e.g. ELECTRON or PROTON) """
    info = Component(EpicsSignalRO, 'info')
    debug = Component(EpicsSignal, 'debug')
    trigger_count = Component(EpicsSignal, 'trigger:count', write_pv='trigger:count:set')
    dt = Component(EpicsSignal, 'dt', write_pv='dt:set')
    trace_dt = Component(EpicsSignalRO, 'trace:dt')
    trace_nt = Component(EpicsSignalRO, 'trace:nt')
    trace_ymin = Component(EpicsSignalRO, 'trace:ymin')
    trace_ymax = Component(EpicsSignalRO, 'trace:ymax')
    trace_yarr = Component(EpicsSignalRO, 'trace:yarr')
