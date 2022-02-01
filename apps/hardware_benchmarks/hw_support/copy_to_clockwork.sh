#!/bin/bash

# usage: copy_to_clockwork <old_name> <clockwork_dir> <new_name>
#
echo "usage: copy_to_clockwork <old_name> <clockwork_dir> <new_name>"

APP=$1
CLOCKWORK=$2
APP_NAME=$3

echo "$1 being renamed to $3"
echo "Add \"prog ${3}();\" to example_progs.h and modify header in example_progs/${3}_memory.cpp"
echo ""

cp bin/${1}_compute.h ${CLOCKWORK}/${3}_compute.h
cp bin/${1}_compute.json ${CLOCKWORK}/coreir_compute/${3}_compute.json
cp bin/${1}_memory.cpp ${CLOCKWORK}/example_progs/${3}_memory.cpp

echo "#include \"example_progs.h\""
echo "prog ${3}() {"
echo "  prog prg;"
echo "  prg.compute_unit_file = \"${3}_compute.h\";"
echo "  prg.name = \"${3}\";"
