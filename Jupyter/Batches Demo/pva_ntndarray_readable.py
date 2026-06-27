# pva_ntndarray_readable.py
# Written by ChatGPT with help from Scott Feister on June 22, 2026.
#
# Generic Bluesky-readable wrapper for one EPICS pvAccess NTNDArray PV.
#
# This is intentionally not Sidekick-specific.
#
# Example:
#
#     electron_trace = PvaNtNdArrayReadable(
#         source="pva://ELECTRON-DAQ:trace",
#         name="electron_trace",
#         shape=(500,),
#     )
#
# Readable fields:
#
#     <name>-array
#     <name>-uniqueId
#     <name>-timeStamp
#
# Direct packet access:
#
#     packet = await device.get_value()
#     array = np.asarray(packet)
#     unique_id = packet.raw["uniqueId"]

from __future__ import annotations

import asyncio
import time
from collections import OrderedDict
from typing import Any

import numpy as np
from ophyd_async.core import Device
from p4p.client.thread import Context


class PvaNtNdArrayReadable(Device):
    """Bluesky-readable wrapper for one PVA NTNDArray PV."""

    def __init__(
        self,
        source: str,
        name: str = "",
        *,
        shape: tuple[int, ...] | None = None,
        context: Context | None = None,
    ) -> None:
        super().__init__(name=name)

        if not source.startswith("pva://"):
            raise ValueError(f"Expected source to start with 'pva://', got {source!r}")

        self.source = source
        self.pv = source.removeprefix("pva://")
        self.shape = shape
        self._ctx = context if context is not None else Context("pva")

    async def connect(
        self,
        mock: bool = False,
        timeout: float = 5.0,
        force_reconnect: bool = False,
    ) -> None:
        """Verify that the PV can be read."""
        if mock:
            return

        await asyncio.to_thread(self._ctx.get, self.pv, timeout=timeout)

    async def get_value(self) -> Any:
        """Return one atomic p4p NTNDArray-like packet."""
        return await asyncio.to_thread(self._ctx.get, self.pv)

    async def get_array(self) -> np.ndarray:
        """Return the NTNDArray payload as a plain NumPy array."""
        packet = await self.get_value()
        return np.asarray(packet)

    async def read(self) -> OrderedDict[str, dict[str, Any]]:
        """Return normal Bluesky event data.

        This performs one atomic PVA read. The array, uniqueId, and timestamp
        all come from the same NTNDArray packet.
        """
        packet = await self.get_value()
        timestamp = packet_timestamp(packet)

        data: OrderedDict[str, dict[str, Any]] = OrderedDict()

        data[f"{self.name}-array"] = {
            "value": np.asarray(packet),
            "timestamp": timestamp,
        }

        data[f"{self.name}-uniqueId"] = {
            "value": int(packet.raw["uniqueId"]),
            "timestamp": timestamp,
        }

        data[f"{self.name}-timeStamp"] = {
            "value": timestamp,
            "timestamp": timestamp,
        }

        return data

    async def describe(self) -> OrderedDict[str, dict[str, Any]]:
        """Return normal Bluesky descriptor data."""
        shape = self.shape

        if shape is None:
            packet = await self.get_value()
            shape = tuple(np.asarray(packet).shape)

        desc: OrderedDict[str, dict[str, Any]] = OrderedDict()

        desc[f"{self.name}-array"] = {
            "source": self.source,
            "dtype": "array",
            "shape": list(shape),
        }

        desc[f"{self.name}-uniqueId"] = {
            "source": self.source,
            "dtype": "integer",
            "shape": [],
        }

        desc[f"{self.name}-timeStamp"] = {
            "source": self.source,
            "dtype": "number",
            "shape": [],
        }

        return desc


def packet_timestamp(packet: Any) -> float:
    """Return a timestamp for a p4p NTNDArray packet."""
    timestamp = getattr(packet, "timestamp", None)

    if timestamp is not None:
        return float(timestamp)

    return time.time()