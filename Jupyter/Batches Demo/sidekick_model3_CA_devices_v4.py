"""
sidekick_model3_ophyd_async.py

Ophyd-async devices for the Channel Access PVs of Sidekick Model 3.

Original ophyd version created by Scott Feister on 2026-06-22.
Converted to ophyd-async by ChatGPT with help from Scott Feister on 2026-06-26.

This file was checked against these EPICS database files:

    teensypulse.db
    lillaser.db
    teensydiode.db

Purpose
-------
Define clean Bluesky/Ophyd device classes for the Sidekick Model 3 EPICS PVs.
The goal is that Bluesky plans can stay simple: set devices, trigger/read devices,
and let the device definitions contain the PV naming details.

Typical use
-----------
    from ophyd_async.core import init_devices
    from sidekick_model3_ophyd_async import PulseGenerator, LilLaser, Diode

    with init_devices():
        pulsegen = PulseGenerator("PULSEGEN:", name="pulsegen")
        laser = LilLaser("LASER:", name="laser")
        electron = Diode("ELECTRON:", name="electron")
        proton = Diode("PROTON:", name="proton")

Design notes
------------
This uses the ophyd-async declarative EPICS style:

    signal: Annotated[
        SignalRW[type],
        PvSuffix(read_suffix="readback", write_suffix="setpoint"),
        Format.CONFIG_SIGNAL,
    ]

Signal type convention:

    SignalR[T]
        Read-only EPICS signal.

    SignalW[T]
        Write-only EPICS signal.

    SignalRW[T]
        Read/write EPICS signal with either one PV or separate read/write PVs.

Format convention:

    Format.CONFIG_SIGNAL
        Slow-changing device state or metadata.

    Format.HINTED_UNCACHED_SIGNAL
        Main run data or status that should be read fresh from EPICS.

Important EPICS database details
--------------------------------
The ``debug`` records in the current database files are ``stringin`` records.
They are therefore modeled here as read-only ``SignalR[str]`` signals, not as
writable signals.

The ``powers:dt`` field in ``lillaser.db`` is a ``longin/longout`` pair with
engineering units of microseconds. It is therefore modeled as ``int``, not
``float``.

The pulse-generator channel delays are ``int64in/int64out`` records with
engineering units of microseconds. They are therefore modeled as ``int``, not
``float``.

The array records are ``aai/aao`` records with ``NELM=100`` and ``FTVL=USHORT``.
Channel Access presents these arrays to ophyd-async as ``Array1D[np.int32]``.
"""

from __future__ import annotations

from typing import Annotated as A

import numpy as np

from ophyd_async.core import (
    Array1D,
    AsyncStatus,
    SignalR,
    SignalRW,
    SignalW,
    StandardReadable,
    set_and_wait_for_other_value,
)
from ophyd_async.core import StandardReadableFormat as Format
from ophyd_async.epics.core import EpicsDevice, PvSuffix


class LaserPowers(StandardReadable, EpicsDevice):
    """
    Laser power waveform with setpoint/readback completion.

    Expected EPICS prefix
    ---------------------
    LASER:

    EPICS PVs
    --------
    readback:
        LASER:powers

    setpoint:
        LASER:powers:set

    Bluesky behavior
    ----------------
    ``bps.mv(laser.powers, waveform)`` writes the waveform to ``powers:set``
    and waits until the ``powers`` readback matches before the plan continues.
    """

    readback: A[
        SignalR[Array1D[np.int32]],
        PvSuffix("powers"),
        Format.HINTED_UNCACHED_SIGNAL,
    ]

    setpoint: A[
        SignalW[Array1D[np.int32]],
        PvSuffix("powers:set"),
    ]

    @AsyncStatus.wrap
    async def set(self, value: Array1D[np.int32]) -> None:
        target = np.asarray(value, dtype=np.int32)

        def matches(readback: Array1D[np.int32]) -> bool:
            actual = np.asarray(readback, dtype=np.int32)
            return actual.shape == target.shape and np.array_equal(actual, target)

        await set_and_wait_for_other_value(
            set_signal=self.setpoint,
            set_value=target,
            match_signal=self.readback,
            match_value=matches,
        )


class PulseGenerator(StandardReadable, EpicsDevice):
    """
    Sidekick Model 3 Teensy pulse generator.

    Expected EPICS prefix
    ---------------------
    PULSEGEN:

    Backing database
    ----------------
    teensypulse.db

    Unit convention
    ---------------
    Channel delays are integer microseconds in the EPICS database.
    """

    # Common read-only string records.
    info: A[
        SignalR[str],
        PvSuffix("info"),
        Format.CONFIG_SIGNAL,
    ]

    debug: A[
        SignalR[str],
        PvSuffix("debug"),
        Format.CONFIG_SIGNAL,
    ]

    # Operational status/control.
    trigger_count: A[
        SignalRW[int],
        PvSuffix(read_suffix="trigger:count", write_suffix="trigger:count:set"),
        Format.HINTED_UNCACHED_SIGNAL,
    ]

    output_enabled: A[
        SignalRW[bool],
        PvSuffix(read_suffix="output:enabled", write_suffix="output:enabled:set"),
        Format.CONFIG_SIGNAL,
    ]

    reprate: A[
        SignalRW[float],
        PvSuffix(read_suffix="reprate", write_suffix="reprate:set"),
        Format.CONFIG_SIGNAL,
    ]

    # Channel delay setpoints/readbacks. EPICS units: microseconds.
    ch1_delay: A[
        SignalRW[int],
        PvSuffix(read_suffix="CH1:delay", write_suffix="CH1:delay:set"),
        Format.CONFIG_SIGNAL,
    ]

    ch2_delay: A[
        SignalRW[int],
        PvSuffix(read_suffix="CH2:delay", write_suffix="CH2:delay:set"),
        Format.CONFIG_SIGNAL,
    ]

    ch3_delay: A[
        SignalRW[int],
        PvSuffix(read_suffix="CH3:delay", write_suffix="CH3:delay:set"),
        Format.CONFIG_SIGNAL,
    ]

    ch4_delay: A[
        SignalRW[int],
        PvSuffix(read_suffix="CH4:delay", write_suffix="CH4:delay:set"),
        Format.CONFIG_SIGNAL,
    ]


class LilLaser(StandardReadable, EpicsDevice):
    """
    Sidekick Model 3 Teensy LilLaser.

    Expected EPICS prefix
    ---------------------
    LASER:

    Backing database
    ----------------
    lillaser.db

    Unit convention
    ---------------
    ``powers_dt`` is integer microseconds in the EPICS database.
    ``powers`` is a 100-element unsigned-short array in ADC units.
    """

    def __init__(self, prefix: str, name: str = "") -> None:
        # Explicit child device. This avoids using PvSuffix("") as a child-device
        # annotation, which is not accepted by this ophyd-async version.
        self.powers = LaserPowers(prefix, name="powers")
        super().__init__(prefix, name=name)

    # Common read-only string records.
    info: A[
        SignalR[str],
        PvSuffix("info"),
        Format.CONFIG_SIGNAL,
    ]

    debug: A[
        SignalR[str],
        PvSuffix("debug"),
        Format.CONFIG_SIGNAL,
    ]

    # Operational status/control.
    trigger_count: A[
        SignalRW[int],
        PvSuffix(read_suffix="trigger:count", write_suffix="trigger:count:set"),
        Format.HINTED_UNCACHED_SIGNAL,
    ]

    output_enabled: A[
        SignalRW[bool],
        PvSuffix(read_suffix="output:enabled", write_suffix="output:enabled:set"),
        Format.CONFIG_SIGNAL,
    ]

    # Laser waveform metadata.
    powers_dt: A[
        SignalRW[int],
        PvSuffix(read_suffix="powers:dt", write_suffix="powers:dt:set"),
        Format.CONFIG_SIGNAL,
    ]

    powers_nt: A[
        SignalR[int],
        PvSuffix("powers:nt"),
        Format.CONFIG_SIGNAL,
    ]

class Diode(StandardReadable, EpicsDevice):
    """
    Sidekick Model 3 Teensy photodiode device.

    Expected EPICS prefixes
    -----------------------
    ELECTRON:
    PROTON:

    Backing database
    ----------------
    teensydiode.db

    Unit convention
    ---------------
    ``dt`` and ``trace_dt`` are floating-point seconds.
    ``trace_nt`` is an integer number of trace points.
    ``trace_yarr`` is a 100-element unsigned-short array in ADC units.
    """

    # Common read-only string records.
    info: A[
        SignalR[str],
        PvSuffix("info"),
        Format.CONFIG_SIGNAL,
    ]

    debug: A[
        SignalR[str],
        PvSuffix("debug"),
        Format.CONFIG_SIGNAL,
    ]

    # Operational status/control.
    trigger_count: A[
        SignalRW[int],
        PvSuffix(read_suffix="trigger:count", write_suffix="trigger:count:set"),
        Format.HINTED_UNCACHED_SIGNAL,
    ]

    dt: A[
        SignalRW[float],
        PvSuffix(read_suffix="dt", write_suffix="dt:set"),
        Format.CONFIG_SIGNAL,
    ]

    # Trace metadata.
    trace_dt: A[
        SignalR[float],
        PvSuffix("trace:dt"),
        Format.CONFIG_SIGNAL,
    ]

    trace_nt: A[
        SignalR[int],
        PvSuffix("trace:nt"),
        Format.CONFIG_SIGNAL,
    ]

    trace_ymin: A[
        SignalR[float],
        PvSuffix("trace:ymin"),
        Format.CONFIG_SIGNAL,
    ]

    trace_ymax: A[
        SignalR[float],
        PvSuffix("trace:ymax"),
        Format.CONFIG_SIGNAL,
    ]

    # Main trace data.
    trace_yarr: A[
        SignalR[Array1D[np.int32]],
        PvSuffix("trace:yarr"),
        Format.HINTED_UNCACHED_SIGNAL,
    ]