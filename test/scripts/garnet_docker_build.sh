#TRAVIS_COMMIT=asgfdsgadsf
CMD_STRING="docker exec -i halide-hw-distro bash -c \"cd GarnetFlow/scripts; ./build_halide_distrib.sh StanfordAHA/Halide-to-Hardware $TRAVIS_COMMIT\""

echo "CMD_STRING is..."
echo $CMD_STRING
#exit
docker pull keyiz/garnet-flow-cad
docker run -itd --name halide-hw-distro keyiz/garnet-flow-cad bash
# TODO: Change this script to use travis environment variables
docker exec -i halide-hw-distro bash -c "git clone --branch dhuff_migrate_coreir_build_halide_from_scratch --depth 1 https://github.com/StanfordAHA/GarnetFlow.git"
eval $CMD_STRING
#docker exec -i halide-hw-distro bash -c "cd GarnetFlow/scripts/; ./build_halide_distrib.sh StanfordAHA/Halide-to-Hardware $COMMIT"
# StanfordAHA/Halide-to-Hardware hls_hwbuffer_codegen_garnetflow_updates"
docker exec -i halide-hw-distro bash -c "cd GarnetFlow/scripts/Halide-to-Hardware; ./test/scripts/compare_app.sh pointwise"
echo 'Pointwise result?'
echo $?
docker exec -i halide-hw-distro bash -c "cd GarnetFlow/scripts/Halide-to-Hardware; ./test/scripts/compare_test.sh conv_3_3"
echo 'Conv result?'
echo $?
docker cp halide-hw-distro:/GarnetFlow/scripts/Halide-to-Hardware/halide_distrib.tgz ./halide_distrib.tgz

