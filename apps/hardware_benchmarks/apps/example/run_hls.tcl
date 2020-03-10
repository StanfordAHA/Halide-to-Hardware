# Environment variables needed
set HALIDE_PATH	$env(HALIDE_DIR)
set RUN_PATH 	$env(RUN_DIR)
set HLS_KERNEL 	$env(KERNEL)

set HLS_INC_FLAGS "-I${HALIDE_PATH}/include -I${HALIDE_PATH}/apps/hardware_benchmarks/hw_support -I${RUN_PATH}"
set INC_FLAGS "-I${HALIDE_PATH}/include -I${HALIDE_PATH}/tools -I${HALIDE_PATH}/apps/hardware_benchmarks/hw_support -I${RUN_PATH}"
set LD_FLAGS "${RUN_PATH}/${HLS_KERNEL}.a -L${HALIDE_PATH}/lib -lpthread -ldl -lpng -ljpeg -lHalide"

# creating the project and seting up the environtment
open_project vhls_prj
set_top vhls_target
add_files ${RUN_PATH}/vhls_target.cpp -cflags "-std=c++0x $HLS_INC_FLAGS"
add_files -tb ${RUN_PATH}/${HLS_KERNEL}_vhls.cpp -cflags "-std=c++0x $INC_FLAGS"
add_files -tb ${RUN_PATH}/../run.cpp -cflags "-std=c++0x $INC_FLAGS"

# Target FPGA and clock constraints
open_solution "solution1"
set_part {xc7z020clg484-1}
create_clock -period 10 -name default
#config_rom_infer -array_size_threshold 1024
config_bind -effort high
config_schedule -effort high

# C simluation
csim_design -O -compiler clang -ldflags ${LD_FLAGS} -argv $env(RUN_ARGS)

# C-to-Verilog synthesis
csynth_design

# Co-simulation (i.e. verifying RTL using C testbench)
cosim_design -O -compiler clang -ldflags ${LD_FLAGS} -argv $env(RUN_ARGS)

# export the RTL design as a Xilix IP Catalog
#export_design -format ip_catalog
#export_design -evaluate verilog -format ip_catalog

exit
