#!/bin/sh

IN=$1
OUT=$2

/usr/bin/wgs84togpx --in=$1 --out=$1.gpx && perl /usr/bin/gpx2png.pl -o $2 $1.gpx
