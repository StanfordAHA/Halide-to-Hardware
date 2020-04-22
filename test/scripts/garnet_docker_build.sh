echo 'Pulling docker image'
docker pull keyiz/garnet-flow-cad

echo 'Running docker container'
docker run -itd --name halide-hw-distro keyiz/garnet-flow-cad bash

echo 'Cloning GarnetFlow into container'
docker exec -i halide-hw-distro bash -c "git clone https://github.com/StanfordAHA/GarnetFlow.git;"

echo 'Creating Halide-to-Hardware directory'
docker exec -i halide-hw-distro bash -c "mkdir /GarnetFlow/scripts/Halide-to-Hardware/"

echo 'Copying Halide-to-Hardware into docker container...'
docker cp ./ halide-hw-distro:/GarnetFlow/scripts/Halide-to-Hardware

docker exec -i halide-hw-distro bash -c "ls /GarnetFlow/scripts"
docker exec -i halide-hw-distro bash -c "ls /GarnetFlow/scripts/Halide-to-Hardware/"

echo 'Running Halide build...'
# Build coreir
docker exec -i halide-hw-distro bash -c "cd /GarnetFlow/scripts; 	git clone --depth 1 --branch fix_simplugin https://github.com/rdaly525/coreir coreir"
docker exec -i halide-hw-distro bash -c "cd /GarnetFlow/scripts/coreir/build && cmake .. && make -j4"
# Build BufferMapping
docker exec -i halide-hw-distro bash -c "cd /GarnetFlow/scripts; git clone --branch new_config --depth 1 https://github.com/joyliu37/BufferMapping"
docker exec -i halide-hw-distro bash -c "cd /GarnetFlow/scripts/BufferMapping/cfunc; make lib"
docker exec -i halide-hw-distro bash -c "ls /GarnetFlow/scripts/BufferMapping"
# Build LLVM
docker exec -i halide-hw-distro bash -c "export LLVM_VERSION=7.0.0; cd /GarnetFlow/scripts/Halide-to-Hardware; ./test/scripts/install_travis.sh; cp -r /llvm /usr/lib/llvm-7"
docker exec -i halide-hw-distro bash -c "apt-get update"
# Run rest of GarnetFlow build script
docker exec -i halide-hw-distro bash -c "cd /GarnetFlow/scripts; ./build_halide_distrib.sh"

# Run the testbench in docker, copy the result to the local travis machine, and
# then stop the script if the testbench failed.
echo 'Running testbench...'
docker exec -i halide-hw-distro bash -c "cd /GarnetFlow/scripts/Halide-to-Hardware; ./test/scripts/garnet_docker_testbench.sh"
echo 'Extracting testbench files...'
docker cp halide-hw-distro:/GarnetFlow/scripts/Halide-to-Hardware/tb_res.txt ./tb_res.txt
test_res=`cat tb_res.txt`
echo 'test_res...'
echo $test_res
if [ $test_res = "passed" ]
then
  echo 'All Tests Passed'
else
  echo 'Tests FAILED'
  exit 1
fi

echo 'Copying built distribution to travis'
docker cp halide-hw-distro:/GarnetFlow/scripts/Halide-to-Hardware/halide_distrib.tgz ./halide_distrib.tgz
