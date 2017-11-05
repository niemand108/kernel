#!/bin/sh

module="niemand"
device="niemand"
mode="664"

if grep -q '^staff:' /etc/group; then
    group="staff"
else
    group="wheel"
fi

/sbin/insmod ./$module.ko $* || exit 1

major=$(awk "\$2==\"$module\" {print \$1}" /proc/devices)

rm -f /dev/${device}0
mknod /dev/${device}0 c $major 0
ln -sf ${device}0 /dev/${device}
chgrp $group /dev/${device}0
chmod $mode  /dev/${device}0
