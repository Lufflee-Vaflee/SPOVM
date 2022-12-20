#!/bin/bash

rm -r files
mkdir -p files
for ((i = 1; i <= $1; i++))
do
touch "./files/f$i"
dd if=/dev/urandom of=./files/f$i bs=1024 count=1024
done