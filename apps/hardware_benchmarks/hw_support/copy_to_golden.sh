#!/bin/bash

# usage: copy_to_clockwork <file_dir> <golden_dir> <new_name>
#
echo "usage: copy_to_clockwork <old_dir> <golden_dir> <new_name>"

APP=$1
GOLDEN=$2
APP_NAME=$3

echo "$1 being renamed to $3"
echo "Copying files to golden"

cp bin/${1}_compute.h ${GOLDEN}/${3}_compute.h
cp bin/${1}_compute.json ${GOLDEN}/${3}_compute.json
cp bin/${1}_memory.cpp ${GOLDEN}/${3}_memory.cpp
cp bin/design_top.json ${GOLDEN}/${3}_design_top.json
