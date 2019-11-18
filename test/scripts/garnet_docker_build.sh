docker pull keyiz/garnet-flow-cad
docker run -itd --name halide-hw-distro keyiz/garnet-flow-cad bash
# TODO: Change this script to use travis environment variables
docker exec -i halide-hw-distro bash -c "git clone --branch dhuff_migrate_coreir_build_halide_from_scratch --depth 1 https://github.com/StanfordAHA/GarnetFlow.git"

#cmd="bash -c \"echo $FOO, $bar\""
#final_cmd="docker exec -it $container $cmd"

#echo "running command: \"$final_cmd\""
#eval $final_cmd
- docker exec -i halide-hw-distro bash -c "cd GarnetFlow/scripts/; ./build_halide_distrib.sh StanfordAHA/Halide-to-Hardware hls_hwbuffer_codegen_garnetflow_updates"
# StanfordAHA/Halide-to-Hardware hls_hwbuffer_codegen_garnetflow_updates"
docker exec -i halide-hw-distro bash -c "cd GarnetFlow/scripts/Halide-to-Hardware; ./test/scripts/compare_app.sh pointwise"
docker exec -i halide-hw-distro bash -c "cd GarnetFlow/scripts/Halide-to-Hardware; ./test/scripts/compare_test.sh conv_3_3"
docker cp halide-hw-distro:/GarnetFlow/scripts/Halide-to-Hardware/halide_distrib.tgz ./halide_distrib.tgz

