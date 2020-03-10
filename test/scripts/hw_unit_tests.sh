# Do not do this unless we are on darwin
if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "On mac"
  source ./test/scripts/mac_settings.sh
fi
if ./test/scripts/rebuild.sh; then
  echo "Built"
else
  echo "Build failed"
  exit 1
fi
./test/scripts/run_hw_unit_tests.sh
