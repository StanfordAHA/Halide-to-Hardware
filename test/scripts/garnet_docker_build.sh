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

# Run the testbench in docker, copy the result to the local travis machine, and
# then stop the script if the testbench failed.
docker exec -i halide-hw-distro bach -c "cd Halide-to-Hardware; ./test/scripts/garnet_docker_testbench.sh"
docker cp halide-hw-distro:/GarnetFlow/scripts/Halide-to-Hardware/tb_res.txt ./tb_res.txt
test_res=`cat tb_res.txt`
if [ test_res = "passed" ]
then
  echo 'All Tests Passed'
else
  echo 'Tests FAILED'
  exit 1
fi

echo 'Copying built distribution to travis'
docker cp halide-hw-distro:/GarnetFlow/scripts/Halide-to-Hardware/halide_distrib.tgz ./halide_distrib.tgz
