# pva_ntndarray_source.py
# Written by ChatGPT with help from Scott Feister on 2026-06-18.
#
# Simple Bluesky-readable source for one EPICS pvAccess NTNDArray PV.
#
# This reads one atomic PVA NTNDArray and emits:
#
#   <name>_trace
#   <name>_uniqueId
#   <name>_timeStamp
#   <name>_<every NTAttribute name>
#
# p4p automatically unwraps NTNDArray into an augmented numpy array.
# That array is the trace data, and array.attrib contains NTAttribute metadata.

from __future__ import annotations

import asyncio
from collections import OrderedDict
from typing import Any

import numpy as np
from ophyd_async.core import Device
from p4p.client.thread import Context


class PvaNtNdArraySource(Device):
    """Bluesky-readable wrapper for one PVA NTNDArray PV."""

    def __init__(self, pv: str, name: str) -> None:
        super().__init__(name=name)

        self.pv = pv
        self.source = f"pva://{pv}"

        # p4p is synchronous, so calls happen through asyncio.to_thread().
        self._ctx = Context("pva")

    async def connect(self, mock: bool = False, timeout: float = 5.0) -> None:
        """Verify that the PV can be read."""
        if mock:
            return

        await asyncio.to_thread(self._ctx.get, self.pv, timeout=timeout)

    async def read(self) -> OrderedDict[str, dict[str, Any]]:
        """Read one atomic NTNDArray and return Bluesky event data."""
        array = await self._get_array()

        timestamp = float(array.timestamp)
        data = OrderedDict()

        # Main trace data.
        data[self._key("trace")] = {
            "value": np.asarray(array),
            "timestamp": timestamp,
        }

        # Built-in timestamp from p4p's NTNDArray wrapper.
        data[self._key("timeStamp")] = {
            "value": timestamp,
            "timestamp": timestamp,
        }

        # Built-in uniqueId, pulled from the raw NTNDArray structure.
        #
        # Keep this small and explicit because uniqueId is not part of
        # array.attrib; it is a top-level NTNDArray field.
        raw = array.raw

        if "uniqueId" in raw:
            data[self._key("uniqueId")] = {
                "value": int(raw["uniqueId"]),
                "timestamp": timestamp,
            }

        if "dataTimeStamp" in raw:
            data[self._key("dataTimeStamp")] = {
                "value": epics_time(raw["dataTimeStamp"]),
                "timestamp": timestamp,
            }

        # User metadata from NTAttribute[].
        for field, value in array.attrib.items():
            data[self._key(field)] = {
                "value": plain_value(value),
                "timestamp": timestamp,
            }

        return data

    async def describe(self) -> OrderedDict[str, dict[str, Any]]:
        """Describe the fields returned by read()."""
        array = await self._get_array()

        desc = OrderedDict()

        desc[self._key("trace")] = {
            "source": self.source,
            "dtype": "array",
            "shape": list(array.shape),
        }

        desc[self._key("timeStamp")] = describe_scalar(self.source, "number")

        raw = array.raw

        if "uniqueId" in raw:
            desc[self._key("uniqueId")] = describe_scalar(self.source, "integer")

        if "dataTimeStamp" in raw:
            desc[self._key("dataTimeStamp")] = describe_scalar(self.source, "number")

        for field, value in array.attrib.items():
            desc[self._key(field)] = describe_scalar(
                self.source,
                dtype_for(plain_value(value)),
            )

        return desc

    async def _get_array(self) -> np.ndarray:
        """Read the PVA NTNDArray.

        p4p returns an augmented numpy.ndarray.
        The array data is the trace.
        The NTAttribute metadata is in array.attrib.
        The full raw PVA value is in array.raw.
        """
        return await asyncio.to_thread(self._ctx.get, self.pv)

    def _key(self, field: str) -> str:
        """Make one Bluesky data key."""
        return f"{self.name}_{field}"


def epics_time(t: Any) -> float:
    """Convert EPICS time_t to seconds."""
    return float(t["secondsPastEpoch"]) + 1e-9 * float(t["nanoseconds"])


def plain_value(value: Any) -> Any:
    """Make scalar metadata values easy for Bluesky/Tiled to store."""
    array = np.asarray(value)

    if array.shape == ():
        return array.item()

    return value


def dtype_for(value: Any) -> str:
    """Return a Bluesky dtype string for one metadata value."""
    if isinstance(value, bool):
        return "boolean"

    if isinstance(value, int):
        return "integer"

    if isinstance(value, float):
        return "number"

    return "string"


def describe_scalar(source: str, dtype: str) -> dict[str, Any]:
    """Return a standard Bluesky description for one scalar field."""
    return {
        "source": source,
        "dtype": dtype,
        "shape": [],
    }