"""
array_steps.py

Created by ChatGPT with help from Scott Feister on 2026-06-26.

Custom Bluesky per-step helpers for scans with array-valued motor positions.

Bluesky's default move_per_step() checks whether a motor is already at its
target position using plain ==. That works for scalar motor positions, but it
can fail for NumPy array positions because array == array returns an array of
booleans instead of one True/False value.

This file provides a small replacement that keeps the usual Bluesky structure,
but uses NumPy-safe position comparison. It also provides a per_step factory
that repeats readings at each scan point and records scan_step / scan_rep as
ordinary event data.

Typical use:

    RE(
        bp.list_scan(
            detectors,
            laser.powers,
            pulse_list,
            per_step=array_one_nd_step_with_reps(reps=7),
        )
    )

Note that you can examine source code for the originals that this replaces:
https://blueskyproject.io/bluesky/main/_modules/bluesky/plan_stubs.html#one_nd_step
https://blueskyproject.io/bluesky/main/_modules/bluesky/plan_stubs.html#move_per_step
"""

from typing import Any

import numpy as np

from bluesky import Msg
from bluesky.plan_stubs import trigger_and_read
from bluesky.utils import short_uid as _short_uid
from ophyd.sim import SynSignal


def positions_equal(a: Any, b: Any) -> bool:
    """
    Return True if two motor positions should be considered equal.

    Safe for scalars, lists, tuples, and NumPy arrays.
    """
    return np.array_equal(a, b)


def array_safe_move_per_step(step, pos_cache):
    """
    Inner loop of an N-dimensional step scan without any readings.

    Like Bluesky's move_per_step(), but safe for NumPy array positions.
    """
    yield Msg("checkpoint")
    grp = _short_uid("set")

    for motor, pos in step.items():
        if positions_equal(pos, pos_cache[motor]):
            # This step does not move this motor.
            continue

        yield Msg("set", motor, pos, group=grp)
        pos_cache[motor] = pos

    yield Msg("wait", None, group=grp)


def array_one_nd_step_with_reps(reps=1):
    """
    Return a per_step function for list_scan with array-valued positions.

    The returned function has the standard Bluesky per_step signature:

        per_step(detectors, step, pos_cache)

    Compared with Bluesky's default one_nd_step(), this version:

    - uses array-safe position comparison
    - repeats readings at each scan position
    - adds scan_step and scan_rep fields

    scan_step and scan_rep are zero-based.
    """
    if reps < 1:
        raise ValueError("reps must be >= 1")

    state = {"step": -1, "rep": 0}

    scan_step = SynSignal(lambda: state["step"], name="scan_step")
    scan_rep = SynSignal(lambda: state["rep"], name="scan_rep")

    def one_nd_step(detectors, step, pos_cache, take_reading=None):
        take_reading = trigger_and_read if take_reading is None else take_reading

        state["step"] += 1

        motors = step.keys()
        readables = list(detectors) + list(motors) + [scan_step, scan_rep]

        yield from array_safe_move_per_step(step, pos_cache)

        for rep in range(reps):
            state["rep"] = rep
            yield from take_reading(readables)

    return one_nd_step