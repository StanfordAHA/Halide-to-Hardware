echo 'Running pointwise...'
./test/scripts/compare_app.sh pointwise
pw_res=$?
echo 'Pointwise result:'
echo $pw_res

if [ $pw_res ]; then
  echo 'Pointwise FAILED'
  echo 'FAILED' > tb_res.txt
  exit 1
else
  echo 'pointwise PASSED'
fi

echo 'Running conv_3_3...'
./test/scripts/compare_app.sh conv_3_3 
pw_res=$?
echo 'conv_3_3 result:'
echo $pw_res

if [ $pw_res ]; then
  echo 'Pointwise FAILED'
  echo 'FAILED' > tb_res.txt
  exit 1
else
  echo 'conv_3_3 PASSED'
fi

echo 'passed' > tb_res.txt
