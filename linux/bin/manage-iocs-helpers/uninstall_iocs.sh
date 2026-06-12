#!/usr/bin/env bash
# created by ChatGPT
#
# Stop, disable auto-start, and uninstall a list of EPICS IOCs, using manage-iocs.
#
# Expected input file:
#   iocs.txt
#
# The file should contain one IOC name per line, for example:
#   CAMERA
#   MOTOR
#   PULSEGEN
#
# For each line in iocs.txt, this script runs:
#   sudo manage-iocs stop NAME
#   sudo manage-iocs disable NAME
#   sudo manage-iocs uninstall NAME
#
# This script reads iocs.txt into memory first. That prevents sudo or
# manage-iocs from accidentally consuming lines from iocs.txt as stdin.

mapfile -t iocs < iocs.txt

for name in "${iocs[@]}"; do
    name="${name%$'\r'}"   # remove Windows carriage return, if present
    [[ -z "$name" ]] && continue

    echo "Uninstalling IOC: [$name]"
    sudo manage-iocs stop "$name"
    sudo manage-iocs disable "$name"
    sudo manage-iocs uninstall "$name"
done

