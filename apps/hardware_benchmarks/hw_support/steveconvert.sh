#!/bin/bash -f

# http://stackoverflow.com/questions/29120768/imagemagick-convert-png-16-bit-to-raw

# Usage: steveconvert gray.png gray.raw

VERBOSE=1
if [[ "$1" == "-q" ]]; then
  VERBOSE=0
  shift
fi

input=$1
output=$2

# set up temp dir
tmpdir=/tmp/myconvert.$$; mkdir $tmpdir
tmp=${tmpdir}/tmp.pgm

convert ${input} -depth 8 pgm:${tmp}
# head -2 $tmp | tail -1

nbytes=`head -2 $tmp | tail -1 | awk '{print $1 * $2}'`
# echo $nbytes

tail -c ${nbytes} ${tmp} > ${output}

# To check the result
(head -3 $tmp; cat $output) > $tmpdir/verify.pgm

if [[ $VERBOSE = 1 ]]; then
  echo "Converted $input to $output...to verify result, do:"
  echo "  gwenview $tmpdir/verify.pgm"
fi

