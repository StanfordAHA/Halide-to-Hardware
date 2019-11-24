
echo 'Pulling docker image'
docker pull keyiz/garnet-flow-cad
echo 'Running docker container'
docker run -itd --name halide-hw-distro keyiz/garnet-flow-cad bash
echo 'Cloning GarnetFlow into container'
docker exec -i halide-hw-distro bash -c "git clone --branch dhuff_migrate_coreir_build_halide_from_scratch --depth 1 https://github.com/StanfordAHA/GarnetFlow.git;"

echo 'Running Halide build...'
# Pull the commit we want from github and build it
CMD_STRING="docker exec -i halide-hw-distro bash -c \"cd GarnetFlow/scripts; ./build_halide_distrib.sh StanfordAHA/Halide-to-Hardware $TRAVIS_COMMIT\";"
echo "CMD_STRING is..."
echo $CMD_STRING
eval $CMD_STRING

echo 'Running pointwise...'
docker exec -i halide-hw-distro bash -c "cd GarnetFlow/scripts/Halide-to-Hardware; ./test/scripts/compare_app.sh pointwise; echo $? > pointwise_res.txt"
docker cp halide-hw-distro:/GarnetFlow/scripts/Halide-to-Hardware/pointwise_res.txt ./pointwise_res.txt
echo 'pointwise_res.txt...'
cat pointwise_res.txt
pw_res=`cat pointwise_res.txt`
if [ $pw_res = "0" ]
then
  echo 'Test passed...'
else
  echo 'Pointwise FAILED'
  exit 1
fi

echo 'Pointwise result?'
echo $?

echo 'Running conv_3_3...'
docker exec -i halide-hw-distro bash -c "cd GarnetFlow/scripts/Halide-to-Hardware; ./test/scripts/compare_test.sh conv_3_3;"
echo 'Conv result?'
echo $?

echo 'Copying built distribution to travis'
docker cp halide-hw-distro:/GarnetFlow/scripts/Halide-to-Hardware/halide_distrib.tgz ./halide_distrib.tgz

