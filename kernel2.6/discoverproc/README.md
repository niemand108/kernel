This module find hidden process.

This module is valid for the 2.6 kernel.

Steps to test the module:

  1. make
  2. insmod niemand.ko
  3. cat /proc/niemand # (only print something when there are hidden process)
