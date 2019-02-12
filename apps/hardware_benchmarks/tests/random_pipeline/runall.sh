#!/bin/bash

# Make command components.
PROCESS_COMMAND=design
declare -a TARGET_MACHINES=("cpu")
#declare -a TARGET_MACHINES=("cpu" "coreir")
declare -a DATATYPES=("int" "float")

# Define the applications and number of iterations for each.
declare -a APPLICATIONS=("pointwise" "conv" "up" "down" "hist" "total")
declare -A APP_ITERATIONS
APP_ITERATIONS[pointwise]=5
APP_ITERATIONS[conv]=15
APP_ITERATIONS[up]=5
APP_ITERATIONS[down]=5
APP_ITERATIONS[hist]=5
APP_ITERATIONS[total]=15


for TARGET_MACHINE in ${TARGET_MACHINES[@]}; do
for DATATYPE in ${DATATYPES[@]}; do
for APPLICATION in ${APPLICATIONS[@]}; do
for SEED_VALUE in `seq 1 ${APP_ITERATIONS[$APPLICATION]}`; do
        
    COMMAND_STRING="$PROCESS_COMMAND-$TARGET_MACHINE-$DATATYPE-$APPLICATION-SEED=$SEED_VALUE"
    make -s $PROCESS_COMMAND-$TARGET_MACHINE-$DATATYPE-$APPLICATION SEED=$SEED_VALUE &> /dev/null
    EXIT_CODE=$?

    if [[ $EXIT_CODE = "0" ]]; then
        printf "%-45s \033[0;32m%s\033[0m\n" $COMMAND_STRING "PASSED"
    else
        printf "%-45s \033[0;31m%s\033[0m\n" $COMMAND_STRING "FAILED"
    fi

done
done
done
done
