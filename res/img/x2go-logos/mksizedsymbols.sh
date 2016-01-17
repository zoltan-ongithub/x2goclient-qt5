#!/bin/bash
# Use this script to build typical iconsized PNGs based on a svg sourcefile.
# ----------------------------------------------------------------------------
# Written by Heinz-M. Graesing <heinz-m.graesing@obviously-nice.de>
# (C) 2012-2016 Heinz-M. Graesing GNU GPL v2.0+
# Last updated on: Dec/01/2012 by Heinz-M. Graesing
# ----------------------------------------------------------------------------

usage="mksizedsymbols.sh -f sourcefile.svg"
sizes=( 12 16 32 48 64 128 )


counter="0"
while getopts "f:h" options; do
	case $options in
	f ) if [ -f $OPTARG ] ;then
		file=$OPTARG
			((counter+=1))
		fi
		;;
	h ) echo $usage
		exit 0
		;;
	\? ) echo $usage
		exit 1;;
	esac
done


if [ -e "$file" ];then
	for i in "${sizes[@]}"
		do
			echo $i
		inkscape --without-gui --export-area-page --export-width=$i --export-height=$i  --file=$file --export-png=${file%.*}-$i.png
		done
	exit 0
else
	echo $usage
	exit 1
fi

