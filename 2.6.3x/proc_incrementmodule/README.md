Increment Module with /proc filesystem: in each read increments a intern variable in one.

This module is valid for the 2.6 kernel.

Steps to load the module:

  1. make
  2. ./load.sh (you need to have execute permission +x)

Steps to download the module:

  1. ./unload.sh (you need to have execute permission +x)

Writting /proc:
  echo 12121212 > /proc/niemand
  
Reading /proc:
  cat /proc/niemand
