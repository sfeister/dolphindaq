"""
Simple run summary table for a Tiled catalog of Bluesky runs.

Created by ChatGPT with help from Scott Feister on 2026-06-25.

Goal
----
Make a human-readable table of runs from a Tiled catalog.

This is meant for Jupyter use, where the default Tiled browser view may show
mostly UID strings. This function gives you a practical run logbook table with
times, scan IDs, plans, metadata, stream names, field names, and short UIDs.

Typical use
-----------
    df = summarize_runs(tiled_client)

    df = summarize_runs(
        tiled_client,
        since="2026-06-25 09:00",
        until="2026-06-25 17:00",
    )

    df = summarize_runs(
        tiled_client,
        last=20,
        contains="proton",
    )

    df = summarize_runs(
        tiled_client,
        filters={"project": "sidekick", "system": "model3"},
    )

Notes
-----
This function is intentionally a browser/summary helper, not an archive/export
tool. It does not load the actual data arrays unless detail="shapes" is used,
and even then it only tries to inspect shapes.
"""

from datetime import datetime
from zoneinfo import ZoneInfo

import numpy as np
import pandas as pd
from bluesky_tiled_plugins.queries import TimeRange


def summarize_runs(
    catalog,
    since=None,
    until=None,
    last=None,
    timezone="America/Los_Angeles",
    metadata_columns=("project", "system", "purpose", "sample", "operator", "notes"),
    filters=None,
    contains=None,
    detail="fields",
    newest_first=True,
):
    """
    Summarize Bluesky runs in a Tiled catalog.

    Parameters
    ----------
    catalog
        Tiled catalog containing Bluesky runs.

    since : str, optional
        Start time.

        Examples:
            "2026-06-25"
            "2026-06-25 09:00"
            "2026-06-25T09:00:00"

        If omitted, include runs from the beginning of the catalog.

    until : str, optional
        End time.

        If omitted, include runs through the present.

    last : int, optional
        If given, return only the last N runs after filtering and sorting.

        Example:
            last=20

    timezone : str, optional
        Time zone for interpreting `since` and `until`.

    metadata_columns : sequence of str, optional
        Metadata keys to include as table columns. These are pulled from the
        Run Start metadata.

    filters : dict, optional
        Exact-match metadata filters applied after collecting the runs.

        Example:
            filters={"project": "sidekick", "system": "model3"}

        These check the DataFrame columns, so they can include built-in columns
        like "plan_name" or custom metadata columns like "purpose".

    contains : str, optional
        Simple text search applied after collecting the runs.

        Searches across common human-readable columns such as plan_name,
        purpose, sample, operator, notes, uid, uid_short, streams, and
        primary_fields.

    detail : {"basic", "fields", "shapes"}, optional
        Amount of run structure to include.

        "basic":
            Only run metadata columns.

        "fields":
            Include stream names and primary stream field names.

        "shapes":
            Also try to include primary stream field shapes. This may be slower
            or may fail for some Tiled objects.

    newest_first : bool, optional
        If True, show newest runs first.

    Returns
    -------
    pandas.DataFrame
        One row per run.
    """
    if detail not in {"basic", "fields", "shapes"}:
        raise ValueError("detail must be one of: 'basic', 'fields', 'shapes'")

    tz = ZoneInfo(timezone)

    if since is None and until is None:
        results = catalog
    else:
        since_ts = _to_timestamp(since, tz) if since is not None else None
        until_ts = _to_timestamp(until, tz) if until is not None else None

        results = catalog.search(
            TimeRange(
                since=since_ts,
                until=until_ts,
                timezone=timezone,
            )
        )

    rows = []

    for key, run in results.items():
        md = getattr(run, "metadata", {})
        start = md.get("start", {})
        stop = md.get("stop", {})

        uid = start.get("uid", key)
        start_time = start.get("time")
        stop_time = stop.get("time")

        row = {
            "time": _format_time(start_time, tz),
            "scan_id": start.get("scan_id"),
            "plan_name": start.get("plan_name"),
            "ok": stop.get("exit_status") == "success",
            "exit_status": stop.get("exit_status"),
            "duration_s": _duration(start_time, stop_time),
            "uid_short": _short_uid(uid),
            "uid": uid,
        }

        for name in metadata_columns:
            row[name] = start.get(name)

        if detail in {"fields", "shapes"}:
            stream_names = _safe_keys(run)
            row["streams"] = ", ".join(stream_names)
            row["num_streams"] = len(stream_names)

            primary = _safe_get(run, "primary")
            primary_fields = _safe_keys(primary) if primary is not None else []

            row["primary_fields"] = ", ".join(primary_fields)
            row["num_primary_fields"] = len(primary_fields)

        if detail == "shapes":
            primary = _safe_get(run, "primary")
            row["primary_shapes"] = _summarize_shapes(primary)

        rows.append(row)

    df = pd.DataFrame(rows)

    if len(df) == 0:
        return df

    df = _apply_filters(df, filters)
    df = _apply_contains(df, contains)

    if "time" in df.columns:
        df = df.sort_values("time", ascending=not newest_first)

    if last is not None:
        df = df.head(last)

    return df.reset_index(drop=True)


def _to_timestamp(value, tz):
    """
    Convert a date/time string to a Unix timestamp.

    Date-only strings are interpreted as local midnight.
    """
    dt = datetime.fromisoformat(value)

    if dt.tzinfo is None:
        dt = dt.replace(tzinfo=tz)

    return dt.timestamp()


def _format_time(timestamp, tz):
    """
    Convert a Unix timestamp to a readable local time string.
    """
    if timestamp is None:
        return None

    return datetime.fromtimestamp(timestamp, tz).strftime("%Y-%m-%d %H:%M:%S")


def _duration(start_time, stop_time):
    """
    Compute run duration in seconds.
    """
    if start_time is None or stop_time is None:
        return None

    return round(stop_time - start_time, 3)


def _short_uid(uid, length=8):
    """
    Return a short UID for display.
    """
    if uid is None:
        return None

    return str(uid)[:length]


def _safe_keys(node):
    """
    Safely list keys from a Tiled node-like object.
    """
    if node is None:
        return []

    try:
        return [str(key) for key in node]
    except Exception:
        return []


def _safe_get(node, key):
    """
    Safely get a child from a Tiled node-like object.
    """
    try:
        return node[key]
    except Exception:
        return None


def _summarize_shapes(stream):
    """
    Try to summarize shapes of fields in a stream.

    This is best-effort. If shape information is not available cheaply, it may
    try np.asarray(field).shape, which could load data.
    """
    if stream is None:
        return None

    pieces = []

    for field_name in _safe_keys(stream):
        field = _safe_get(stream, field_name)

        shape = None

        try:
            shape = getattr(field, "shape", None)
        except Exception:
            shape = None

        if shape is None:
            try:
                shape = np.asarray(field).shape
            except Exception:
                shape = "?"

        pieces.append(f"{field_name}: {shape}")

    return "; ".join(pieces)


def _apply_filters(df, filters):
    """
    Apply exact-match filters to DataFrame columns.
    """
    if not filters:
        return df

    out = df

    for column, expected in filters.items():
        if column not in out.columns:
            continue

        out = out[out[column] == expected]

    return out


def _apply_contains(df, contains):
    """
    Apply a simple case-insensitive text search across useful columns.
    """
    if not contains:
        return df

    needle = str(contains).lower()

    search_columns = [
        "time",
        "scan_id",
        "plan_name",
        "purpose",
        "sample",
        "operator",
        "notes",
        "exit_status",
        "uid_short",
        "uid",
        "streams",
        "primary_fields",
        "primary_shapes",
    ]

    search_columns = [col for col in search_columns if col in df.columns]

    if not search_columns:
        return df

    mask = pd.Series(False, index=df.index)

    for col in search_columns:
        mask = mask | df[col].astype(str).str.lower().str.contains(needle, na=False)

    return df[mask]