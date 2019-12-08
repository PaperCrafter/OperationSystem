#!/bin/bash

QEMU=qemu-system-x86_64
IMG_PATH=/home/bobgil/Desktop/MintOS64/Disk.img
MEM_SIZE=64

$QEMU -L . -m $MEM_SIZE -fda $IMG_PATH -localtime -M pc