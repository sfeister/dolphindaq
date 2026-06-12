#!/usr/bin/env bash
# created by ChatGPT
#
# Install and enable auto-start for a list of EPICS IOCs, using manage-iocs.
#
# Expected input file:
#   iocs.txt
#
# The file should contain one IOC name per line, for example:
#   CAMERA
#   MOTOR
#   TRIGGER
#
# For each line in iocs.txt, this script runs:
#   sudo manage-iocs install NAME
#   sudo manage-iocs enable NAME
#
# This script reads iocs.txt into memory first. That prevents sudo or
# manage-iocs from accidentally consuming lines from iocs.txt as stdin.

mapfile -t iocs < iocs.txt

for name in "${iocs[@]}"; do
    name="${name%$'\r'}"   # remove Windows carriage return, if present
    [[ -z "$name" ]] && continue

    echo "Installing IOC: [$name]"
    sudo manage-iocs install "$name"
    echo "Enabling auto-start for IOC: [$name]"
    sudo manage-iocs enable "$name"
done