#!/bin/bash
cd "$(dirname "$0")"

function usage {
    	echo "usage: $0 [-f program-file] [-d debug-flags] [-v]"
    	echo "  -f program-file	file to be executed by NachOS"
	echo "  -v      turn on verbose mode to print all compiling errors and warnings"
	echo "  -d debug-flags	allow debug messages with corresponding flags to be printed out, see all flags in code/lib/debug.h"
    	exit 1
}

isVerboseMode=false
filename=""

while getopts ":vf:d:" o; do
    case "${o}" in
        v)
            	isVerboseMode=true
            	;;
	f)
		filename=${OPTARG}
		;;
	d)
		debugFlags=" -d ${OPTARG}"
		;;
	u)
		usage
		;;
        *)
            	usage
            	;;
    esac
done

if [ ! "$filename" ]; then
  echo "Arg -f must be provided"
  usage >&2; exit 1
fi

echo "Compiling nachOS 4.0"
cd NachOS-4.0/code/build.linux
if [ "$isVerboseMode" = false ] 
then
	make depend >/dev/null 2>&1
	make >/dev/null 2>&1
else
	make depend
	make
fi

echo "Changing directory to NachOS-4.0/code/test"
cd ../test
echo "Building programs in NachOS-4.0/code/test"

if [ "$isVerboseMode" = false ] 
then
	make >/dev/null 2>&1
else
	make
fi

echo "Executing $filename"
echo "-------------------------------------------"
../build.linux/nachos -rs 1023 -x "$filename" $debugFlags


