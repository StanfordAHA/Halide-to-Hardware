DOCKER=$1
APP=$2

docker cp bin/design_meta_halide.json $1:/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/$2/bin/
docker cp bin/design_top.json $1:/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/$2/bin/
docker cp bin/hw_input_stencil.raw $1:/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/$2/bin/
docker cp bin/hw_output.raw $1:/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/$2/bin/

