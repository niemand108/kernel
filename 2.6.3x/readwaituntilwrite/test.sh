#!/bin/sh

./unload.sh 
./load.sh

echo "Reading before writting (blocking process)"
dd count=1 bs=1 of=/dev/niemand &
echo "Writting procces int /dev/niemand-> Awoke estupid" > /dev/niemand

dmesg | tail

