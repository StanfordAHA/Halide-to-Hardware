# Do not do this unless we are on darwin
source ./test/scripts/mac_settings.sh
if ./test/scripts/rebuild.sh; then
  echo "Built"
else
  echo "Build failed"
  exit 1
fi
./test/scripts/run_hw_unit_tests.sh
