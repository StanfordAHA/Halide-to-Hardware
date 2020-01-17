if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "On mac"
  source ./test/scripts/mac_settings.sh
else
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./bin/
fi

echo 'LD_LIBRARY_PATH =' $LD_LIBRARY_PATH
cd ./apps/hardware_benchmarks/tests/merged_unit_tests/
if make all; then
  echo "Make finished"
else 
  echo "Error: Make failed"
  exit 1
fi

./all-tests
