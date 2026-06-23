# pva_ntndarray_signal.py
# Written by ChatGPT with help from Scott Feister on June 22, 2026.
#
# Tiny async wrapper for one EPICS pvAccess NTNDArray PV.
# Lets us keep the atomic metadata attached to the trace.

from __future__ import annotations

import asyncio
from typing import Any

import numpy as np
from ophyd_async.core import Device
from p4p.client.thread import Context


class PvaNtNdArraySignal(Device):
    """Tiny async reader for one PVA NTNDArray PV."""

    def __init__(self, source: str, name: str = "") -> None:
        super().__init__(name=name)

        if not source.startswith("pva://"):
            raise ValueError(
                f"Expected source to start with 'pva://', got {source!r}"
            )

        self.source = source
        self.pv = source.removeprefix("pva://")
        self._ctx = Context("pva")

    async def connect(
        self,
        mock: bool = False,
        timeout: float = 5.0,
        force_reconnect: bool = False,
    ) -> None:
        """Check that the PV is readable.

        The force_reconnect argument is accepted because ophyd-async passes it
        when connecting child Devices.
        """
        if mock:
            return

        await asyncio.to_thread(self._ctx.get, self.pv, timeout=timeout)

    async def get_value(self) -> Any:
        """Return one atomic p4p NTNDArray-like value."""
        return await asyncio.to_thread(self._ctx.get, self.pv)

    async def get_array(self) -> np.ndarray:
        """Return the trace data as a plain NumPy array."""
        return np.asarray(await self.get_value())