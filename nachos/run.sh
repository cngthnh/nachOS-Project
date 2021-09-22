#!/bin/bash

echo "Compiling nachOS 4.0"
cd NachOS-4.0/code/build.linux
make depend >/dev/null 2>&1
make >/dev/null 2>&1
echo "Changing directory to NachOS-4.0/code/test"
cd ../test
echo "Building programs in NachOS-4.0/code/test"
make >/dev/null 2>&1
echo "Executing $1"
echo "-------------------------------------------"
../build.linux/nachos -x "$1"


