cd ./apps/hardware_benchmarks/tests/merged_unit_tests/
if make all; then
  echo "Make finished"
else 
  echo "Error: Make failed"
  exit 1
fi

lldb ./all-tests
