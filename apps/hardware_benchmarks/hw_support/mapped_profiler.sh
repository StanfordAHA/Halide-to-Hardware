#!/bin/bash
FILE=$1

NUM_PE=`grep -o "cgralib.PE" $FILE | wc -l`
NUM_MEM=`grep -o "cgralib.Mem" $FILE | wc -l`
NUM_IO=`grep -o "cgralib.IO" $FILE | wc -l`

#echo "PE=$NUM_PE  MEM=$NUM_MEM  IO=$NUM_IO"

NUM_ADD=`grep -o "coreir.add" $FILE | wc -l`
NUM_SUB=`grep -o "coreir.sub" $FILE | wc -l`
NUM_MUL=`grep -o "coreir.mul" $FILE | wc -l`
NUM_SHR=`grep -o "coreir.ashr" $FILE | wc -l`
NUM_CONST=`grep -o "coreir.const" $FILE | wc -l`
NUM_ABSD=`grep -o "commonlib.absd" $FILE | wc -l`

NUM_COMMON=`grep -o "commonlib." $FILE | wc -l`
NUM_COREIR=`grep -o "coreir." $FILE | wc -l`
echo "COREIR=$NUM_COREIR CONST=$NUM_CONST COMMON=$NUM_COMMON ABSD=$NUM_ABSD"
