#!/bin/sh
module="niemand"
device="niemand"

/sbin/rmmod $module $* || exit 1

rm -f /dev/${device} /dev/${device}0
