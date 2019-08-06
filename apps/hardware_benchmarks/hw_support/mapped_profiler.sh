#!/bin/bash
FILE=$1

NUM_PE=`grep -o "cgralib.PE" $FILE | wc -l`
NUM_MEM=`grep -o "cgralib.Mem" $FILE | wc -l`
NUM_IO=`grep -o "cgralib.IO" $FILE | wc -l`

echo "PE=$NUM_PE  MEM=$NUM_MEM  IO=$NUM_IO"
