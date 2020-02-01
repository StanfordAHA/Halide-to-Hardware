echo 'Running merged test suite...'
./test/scripts/run_hw_unit_tests.sh
pw_res=$?
echo 'HW Unit test result:'
echo $pw_res

if [ $pw_res != "0" ]; then
  echo 'HW Unit tests FAILED'
  echo 'FAILED' > tb_res.txt
  exit 1
else
  echo 'HW Unit tests PASSED'
fi

echo 'Running pointwise...'
./test/scripts/compare_app.sh pointwise >& pw_res.txt
pw_res=$?
echo 'Pointwise result:'
echo $pw_res

if [ $pw_res != "0" ]; then
  echo 'Pointwise FAILED'
  echo 'FAILED' > tb_res.txt
  exit 1
else
  echo 'pointwise PASSED'
fi

echo 'Running conv_3_3...'
./test/scripts/compare_test.sh conv_3_3 >& conv_res.txt
pw_res=$?
echo 'conv_3_3 result:'
echo $pw_res

if [ $pw_res != "0" ]; then
  echo 'conv_3_3 FAILED'
  echo 'FAILED' > tb_res.txt
  exit 1
else
  echo 'conv_3_3 PASSED'
fi

echo 'passed' > tb_res.txt
