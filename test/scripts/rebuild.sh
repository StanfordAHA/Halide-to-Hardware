if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "On mac"
  source ./test/scripts/mac_settings.sh
fi

make -j
make install
make distrib
