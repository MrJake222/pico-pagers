#!/bin/bash

if [[ $# != 2 && $# != 3 ]]; then
	echo "Usage: $0 <usb bus-port> <project name> [release/debug (default is debug)]"
	echo "find USB bus-ports via lsusb -t"
	echo "project name is for ex. pagers-client"
	echo "type is CMake build type (release/debug). Used for CLion folder"
	echo "Example usage:"
	echo -e "\t./program.sh 1-9 pagers-server"
	echo -e "\t./program.sh 1-11 pagers-client"
	exit 1
fi

busport=$1
proj=$2
type=$3
if [[ $type == "" ]]; then type="debug"; fi

devfile="/sys/bus/usb/devices/$busport/devnum"
if [[ ! -f $devfile ]]; then
	echo "no device detected at $busport"
	exit 2
fi

devnum=$(cat $devfile)
echo "programming device $devnum with project $proj type $type"

picotool load -u -v $proj/cmake-build-$type/$proj.uf2 -f --address $devnum
