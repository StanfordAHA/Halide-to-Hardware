if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "On mac"
  source ./test/scripts/mac_settings.sh
else
  echo 'not on mac'
fi
DEBUG_MODE=0
if [ "$#" -ne 1 ]; then
    echo "Not use debug verbose mode"
else
    if [ "$1" != "verbose" ]; then
        echo "Not recognized cmd: [$1]" 
        exit 1
    fi
    DEBUG_MODE=1
fi
export LD_LIBRARY_PATH=`pwd`/bin
echo 'LD_LIBRARY_PATH =' $LD_LIBRARY_PATH
cd ./apps/hardware_benchmarks/tests/merged_unit_tests/
if make all DEBUG_SIM=$DEBUG_MODE; then
  echo "Make finished"
else 
  echo "Error: Make failed"
  exit 1
fi

./all-tests
