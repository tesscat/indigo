#!/bin/bash
echo $(losetup -a | tail -n 1 | awk '{print $1}' | awk '{print substr($1, 1, length($1)-1)}')
