#!/bin/sh

name="rolldet"

/sbin/rmmod $name
/sbin/insmod ./$name.ko $* || exit 1

rm -f /dev/$name

major=$(cat /proc/devices | grep $name | awk '{print $1}' | head -n 1)
mknod /dev/$name c $major 0

chmod 664 /dev/$name
