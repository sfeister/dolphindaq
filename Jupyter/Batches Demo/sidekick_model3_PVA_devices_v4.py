# Ophyd-async Device for the PVAccess PVs of Sidekick Model 3
# Created by ChatGPT with help from Scott Feister on June 22, 2026
#
# PVs:
#   pva://DIODE-DAQ:trace   -> NumPy array
#   pva://DIODE-DAQ:info    -> string
#
# Notes:
#   - This is intentionally separate from the classic ophyd CA Device.
#   - The trace is an uncached primary reading. Uncached because it's data and constantly updating.
#   - The info string is configuration metadata.

import numpy as np

from ophyd_async.core import StandardReadable, StandardReadableFormat as Format, init_devices
from ophyd_async.epics.core import epics_signal_r
from pva_ntndarray_readable_v2 import PvaNtNdArrayReadable # local file, lets us keep metadata atomically attached to arrays

class DiodePVA(StandardReadable):
    """PVAccess read-only device for PVAccess DIODE-DAQ (e.g. ELECTRON-DAQ, PROTON-DAQ, etc)."""

    def __init__(self, prefix: str = "pva://DIODE-DAQ:", name: str = "") -> None:
        # Do NOT put this inside add_children_as_readables().
        # It is not an ophyd-async SignalR.
        self.trace = PvaNtNdArrayReadable(
            source=prefix + "trace",
            shape=(100,),
        )
        
        # Slow-changing metadata. Include in read_configuration().
        with self.add_children_as_readables(Format.CONFIG_SIGNAL):
            self.info = epics_signal_r(
                str,
                prefix + "info",
            )

        super().__init__(name=name)
