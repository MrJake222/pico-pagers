if [[ $# != 2 ]]; then
	echo "Usage: $0 [root dir for lfs] [openocd|picotool]"
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
	# untested
	picotool load -o 0x101F0000 lfs.bin -f
fi

rm lfs.bin
