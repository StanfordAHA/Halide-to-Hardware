#!/bin/bash
TESTNAME=$1
mkdir -p test_results
#rm -f test_results/results

for unroll in {2..8}; do
let wl=64-2*$unroll
let wh=64+$unroll
#let wl=48
#let wl=72
for (( width=$wl; width<=$wh; width++ )); do
  HALIDE_GEN_ARGS="schedule=3 width=$width myunroll=$unroll"
  COMMAND="$TESTNAME $HALIDE_GEN_ARGS"
  COMMAND="${COMMAND// /,}"
  TEST_FILE="test_results/unroll${unroll}_width${width}.txt"
  echo $COMMAND > $TEST_FILE

  EARLY_EXIT=0
  make mem HALIDE_GEN_ARGS="$HALIDE_GEN_ARGS" &>>$TEST_FILE;
  EXIT_CODE=$?;
  cat bin/design_meta_halide.json >> $TEST_FILE
  if [[ $EXIT_CODE = 0 ]]; then
    printf "%-55s \033[0;32m%s\033[0m\n" $COMMAND "PASSED";
    printf "%-55s %s\n" $COMMAND "PASSED" | cat >> test_results/results;
    printf "%-55s %s\n" $COMMAND "PASSED" | cat >> $TEST_FILE;

    make compare HALIDE_GEN_ARGS="$HALIDE_GEN_ARGS" &>>$TEST_FILE
    make bin/hw_input_stencil.raw bin/hw_output.raw &>> $TEST_FILE
    DISABLE_GP=1 aha map apps/$TESTNAME --width 32 --height 16 &>> $TEST_FILE
    EXIT_CODE=$?;
    if [[ $EXIT_CODE = 0 ]]; then
        printf "%-55s \033[0;32m%s\033[0m\n" "  MAPPING" "  PASS";
        printf "%-55s %s\n" "  MAPPING" "  PASS" | cat >> test_results/results;
        printf "%-55s %s\n" "  MAPPING" "  PASS" | cat >> $TEST_FILE;

        aha glb apps/$TESTNAME --width 32 &>> $TEST_FILE
        cat bin/design_meta.json >> $TEST_FILE
        grep -Fq "APP0-$TESTNAME passed" $TEST_FILE
        EXIT_CODE=$?;
        if [[ $EXIT_CODE = 0 ]]; then
            printf "%-55s \033[0;32m%s\033[0m\n" "  GLB SIM" "  PASS";
            printf "%-55s %s\n" "  GLB SIM" "  PASS" | cat >> test_results/results;
            printf "%-55s %s\n" "  GLB SIM" "  PASS" | cat >> $TEST_FILE;
        else
            printf "%-55s \033[0;31m%s\033[0m\n" "  GLB SIM" "  FAIL";
            printf "%-55s %s\n" "  GLB SIM" "  FAIL" | cat >> test_results/results;
            printf "%-55s %s\n" "  GLB SIM" "  FAIL" | cat >> $TEST_FILE;
        fi
    else
        printf "%-55s \033[0;31m%s\033[0m\n" "  MAPPING" "  FAIL";
        printf "%-55s %s\n" "  MAPPING" "  FAIL" | cat >> test_results/results;
        printf "%-55s %s\n" "  MAPPING" "  FAIL" | cat >> $TEST_FILE;
    fi


    (exit $EXIT_CODE);
  else
    printf "%-55s \033[0;31m%s\033[0m\n" $COMMAND "FAILED";
    #printf "%-55s %s\n" $COMMAND "FAILED" | cat >> test_results/results;
    printf "%-55s %s\n" $COMMAND "FAILED" | cat >> $TEST_FILE;
    (exit $EARLY_EXIT);
  fi
done
done
