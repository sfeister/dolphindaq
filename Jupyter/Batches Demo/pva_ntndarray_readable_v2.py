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
#
# Trigger behavior:
#
#     trigger() waits until the NTNDArray uniqueId changes.
#     This makes Bluesky trigger_and_read([device]) read a fresh packet.

from __future__ import annotations

import asyncio
import time
from collections import OrderedDict
from typing import Any

import numpy as np
from ophyd_async.core import AsyncStatus, Device
from p4p.client.thread import Context


class PvaNtNdArrayReadable(Device):
    """Bluesky-readable wrapper for one PVA NTNDArray PV.

    trigger() is passive. It does not cause acquisition. It waits until the
    PV publishes a packet with a new uniqueId.
    """

    def __init__(
        self,
        source: str,
        name: str = "",
        *,
        shape: tuple[int, ...] | None = None,
        context: Context | None = None,
        trigger_timeout: float = 5.0,
        poll_period: float = 0.005,
    ) -> None:
        super().__init__(name=name)

        if not source.startswith("pva://"):
            raise ValueError(f"Expected source to start with 'pva://', got {source!r}")

        self.source = source
        self.pv = source.removeprefix("pva://")
        self.shape = shape
        self._ctx = context if context is not None else Context("pva")

        self.trigger_timeout = trigger_timeout
        self.poll_period = poll_period

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

    async def get_unique_id(self) -> int:
        """Return the NTNDArray uniqueId from one atomic PVA packet."""
        packet = await self.get_value()
        return int(packet.raw["uniqueId"])

    @AsyncStatus.wrap
    async def trigger(self):
        """Wait for a fresh NTNDArray packet.

        Freshness criterion:
            packet.raw["uniqueId"] changes.

        This is passive. It does not command the DAQ to acquire. It assumes
        some external system causes the next packet to be posted.
        """
        old_unique_id = await self.get_unique_id()
        deadline = time.monotonic() + self.trigger_timeout

        while True:
            new_unique_id = await self.get_unique_id()

            if new_unique_id != old_unique_id:
                return

            if time.monotonic() > deadline:
                raise TimeoutError(
                    f"{self.name or self.source} timed out waiting for fresh "
                    f"NTNDArray packet: uniqueId stayed at {old_unique_id} "
                    f"for {self.trigger_timeout} seconds."
                )

            await asyncio.sleep(self.poll_period)

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