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


class DiodePVA(StandardReadable):
    """PVAccess read-only device for PVAccess DIODE-DAQ (e.g. ELECTRON-DAQ, PROTON-DAQ, etc)."""

    def __init__(self, prefix: str = "pva://DIODE-DAQ:", name: str = "") -> None:
        # Large/fresh per-shot data. Include in read(), but do not rely on cache.
        with self.add_children_as_readables(Format.HINTED_UNCACHED_SIGNAL):
            self.trace = epics_signal_r(
                np.ndarray,
                prefix + "trace",
            )

        # Slow-changing metadata. Include in read_configuration().
        with self.add_children_as_readables(Format.CONFIG_SIGNAL):
            self.info = epics_signal_r(
                str,
                prefix + "info",
            )

        super().__init__(name=name)
