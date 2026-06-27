"""
Export one Tiled run to HDF5.

Created by ChatGPT with help from Scott Feister on 2026-06-25.

Goal
----
This file provides a simple, practical exporter:

    Tiled run -> HDF5 file

The goal is to make a portable HDF5 file that contains:

    - the run UID
    - run-level metadata, stored as JSON
    - all visible streams exposed by Tiled
    - all visible fields inside those streams, when they can be converted
      to NumPy arrays
    - a record of anything that could not be exported

This is intended for moving data to another machine, sharing a run with
students/collaborators, sending data to HPC, or opening the run later with
ordinary HDF5 tools.

This is not intended to be a perfect, lossless clone of the original
Bluesky/Tiled database. Tiled remains the real archive/database. The HDF5 file
is an export product.

Limitations
-----------
This exporter is intentionally simple.

It does not:

    - preserve the full original Bluesky document stream
      such as start, descriptor, event, stop, resource, and datum documents
    - create a NeXus-compliant HDF5 file
    - use a suitcase exporter
    - guarantee that every possible Tiled object can be exported
    - preserve every Python object type exactly
    - handle every possible HDF5 naming edge case
    - compress datasets
    - recursively turn metadata into HDF5 groups

Metadata is saved as JSON text. This is simple and readable, but it is not a
fully typed HDF5 metadata representation.

Fields are exported by trying to convert each visible Tiled field to a NumPy
array. This works well for ordinary numeric arrays and traces. It may fail for
more complicated data structures. Failures are recorded in the HDF5 file under
/export_errors.

Typical use
-----------
    filename = export_run_to_hdf5(
        tiled_client,
        uid,
        f"outputs/run_{uid}.h5",
    )

Default use
-----------
If filename is omitted, the file is saved in the current working directory as:

    run_<uid>.h5
"""

from pathlib import Path
import json

import h5py
import numpy as np


def export_run_to_hdf5(tiled_client, uid, filename=None, overwrite=False):
    """
    Export one Tiled run to an HDF5 file.

    Parameters
    ----------
    tiled_client
        Connected Tiled client.

    uid : str
        Run UID.

    filename : str or Path, optional
        Output HDF5 filename.

        If omitted, the file is saved in the current working directory as:

            run_<uid>.h5

        The filename may include a folder, for example:

            outputs/run_<uid>.h5

        Any missing parent folders are created automatically.

    overwrite : bool, optional
        Whether to overwrite an existing HDF5 file.

        The default is False, which is safer for experimental data. If the file
        already exists and overwrite is False, this function raises
        FileExistsError.

    Returns
    -------
    Path
        Path to the written HDF5 file.

    HDF5 layout
    -----------
    The resulting file has a simple custom layout:

        run_<uid>.h5
        ├── attrs["uid"]
        ├── metadata
        │   └── run
        ├── streams
        │   ├── primary
        │   │   ├── field_1
        │   │   ├── field_2
        │   │   └── ...
        │   ├── baseline
        │   │   └── ...
        │   └── ...
        └── export_errors

    Notes
    -----
    This exporter is best-effort. If a stream or field cannot be exported, the
    exporter keeps going and records the problem in /export_errors.
    """
    if filename is None:
        filename = f"run_{uid}.h5"

    filename = Path(filename)
    filename.parent.mkdir(parents=True, exist_ok=True)

    if filename.exists() and not overwrite:
        raise FileExistsError(f"File already exists: {filename}")

    run = tiled_client[uid]
    export_errors = []

    with h5py.File(filename, "w") as f:
        # Store the most important identifier at the top level.
        f.attrs["uid"] = uid

        # Store run-level metadata as JSON text.
        metadata_group = f.create_group("metadata")
        save_json(metadata_group, "run", getattr(run, "metadata", {}))

        # Store visible stream contents.
        streams_group = f.create_group("streams")

        for stream_name in run:
            try:
                stream = run[stream_name]
            except Exception as exc:
                export_errors.append(
                    {
                        "stream": str(stream_name),
                        "field": None,
                        "error": str(exc),
                    }
                )
                continue

            stream_group = streams_group.create_group(clean_hdf5_name(stream_name))

            for field_name in stream:
                try:
                    field = stream[field_name]
                    data = np.asarray(field)

                    dataset_name = clean_hdf5_name(field_name)
                    stream_group.create_dataset(dataset_name, data=data)

                except Exception as exc:
                    export_errors.append(
                        {
                            "stream": str(stream_name),
                            "field": str(field_name),
                            "error": str(exc),
                        }
                    )

        # Always save the error report, even if it is an empty list.
        save_json(f, "export_errors", export_errors)

    return filename


def save_json(group, name, value):
    """
    Save a Python object as pretty-printed JSON text in an HDF5 dataset.

    This is used for metadata and export errors.

    Parameters
    ----------
    group : h5py.Group or h5py.File
        HDF5 group or file where the JSON dataset will be created.

    name : str
        Name of the dataset to create.

    value
        Python object to save. It should ideally be JSON-serializable. Values
        that JSON does not understand are converted to strings.
    """
    text = json.dumps(value, indent=2, default=str)
    group.create_dataset(name, data=text)


def clean_hdf5_name(name):
    """
    Make a simple HDF5-safe name.

    HDF5 uses "/" as a path separator, so field names containing "/" cannot be
    used directly as dataset names. This function replaces "/" with "_".

    This intentionally does not heavily rename fields. The exporter tries to
    keep names close to the names seen in Tiled.
    """
    return str(name).replace("/", "_")