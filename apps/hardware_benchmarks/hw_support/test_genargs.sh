#!/bin/bash
TESTNAME=$1
mkdir -p test_results
rm -f test_results/results

for unroll in {1..4}; do
let wl=64-2*$unroll
let wh=64+$unroll
for (( width=$wl; width<=$wh; width++ )); do
  HALIDE_GEN_ARGS="schedule=3 width=$width myunroll=$unroll"
  COMMAND="$TESTNAME $HALIDE_GEN_ARGS"
  COMMAND="${COMMAND// /,}"
  TEST_FILE="test_results/width${width}_unroll${unroll}.txt"
  echo $COMMAND > $TEST_FILE
  
  EARLY_EXIT=0
  make mem HALIDE_GEN_ARGS="$HALIDE_GEN_ARGS" &>>$TEST_FILE;
  cat bin/design_meta_halide.json >> $TEST_FILE
  EXIT_CODE=$?; 
  if [[ $EXIT_CODE = 0 ]]; then
    printf "%-55s \033[0;32m%s\033[0m\n" $COMMAND "PASSED";
    printf "%-55s %s\n" $COMMAND "PASSED" | cat >> test_results/results;
    printf "%-55s %s\n" $COMMAND "PASSED" | cat >> $TEST_FILE;
    (exit $EXIT_CODE);
  else
    printf "%-55s \033[0;31m%s\033[0m\n" $COMMAND "FAILED";
    printf "%-55s %s\n" $COMMAND "FAILED" | cat >> test_reuslts/results;
    printf "%-55s %s\n" $COMMAND "FAILED" | cat >> $TEST_FILE;
    (exit $EARLY_EXIT);
  fi
done
done
