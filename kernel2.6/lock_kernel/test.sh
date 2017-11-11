#!/bin/sh

./unload.sh 
./load.sh

echo "This will block you computer for some seconds"

dd count=1 bs=1 if=/dev/niemand

dmesg | tail


