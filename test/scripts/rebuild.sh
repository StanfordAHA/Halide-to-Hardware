if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "On mac"
  source ./test/scripts/mac_settings.sh
fi

make -j
if [[ "$OSTYPE" == "darwin"* ]]; then
  make install
fi
make distrib
