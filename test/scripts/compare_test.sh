if cd ./apps/hardware_benchmarks/tests/$1; then
  echo "Entered dir"
else
  echo "Error, no such test: $1"
  exit 1
fi
make clean
make design-coreir
make compare
