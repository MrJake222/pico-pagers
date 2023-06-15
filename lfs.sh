if [[ $# != 2 && $# != 3 ]]; then
	echo "Usage: $0 [root dir for lfs] [openocd|picotool] [devnum for picotool]"
	exit 1
fi

mklfs -r 256 -p 256 -b 4096 -c 1024 -L 1024 $1 lfs.bin 65536

if [[ $2 == "openocd" ]]; then
	/usr/local/bin/openocd \
		-s /usr/local/share/openocd/scripts \
		-f ../rp2040_clion.cfg \
		-c "tcl_port disabled" \
		-c "gdb_port disabled" \
		-c "tcl_port disabled" \
		-c 'program lfs.bin reset exit 0x101F0000'

elif [[ $2 == "picotool" ]]; then
	if [[ $3 != "" ]]; then
		busport=$3
		devfile="/sys/bus/usb/devices/$busport/devnum"
		if [[ ! -f $devfile ]]; then
	        	echo "no device detected at $busport"
	        	exit 2
		fi

		devnum=$(cat $devfile)
		echo "programming device $devnum"
		picotool load -o 0x101F0000 lfs.bin -f --address $devnum
	else
		picotool load -o 0x101F0000 lfs.bin -f
	fi
fi

rm lfs.bin
