include ../../hw_support/Makefile.inc

TESTNAME = handcrafted_ub_dilated_conv
USE_COREIR_VALID ?= 1

include ../../hw_support/hardware_targets.mk

# these are already defined, and don't need to be rebuilt
$(BIN)/design_top.json:
$(BIN)/output_cpu.png:

images: process.cpp $(HWSUPPORT)/$(BIN)/hardware_process_helper.o $(HWSUPPORT)/$(BIN)/coreir_interpret.o
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -I$(BIN) -I$(HWSUPPORT) -I$(HWSUPPORT)/xilinx_hls_lib_2015_4 -Wall $(HLS_PROCESS_CXX_FLAGS)  -O3 $^ -o bin/process $(LDFLAGS) $(IMAGE_IO_FLAGS) 
	$(BIN)/process images

clean:
	mv -t . bin/design_top.json bin/output_cpu.png
	rm bin/*
	mv -t bin design_top.json output_cpu.png

# Usage:
#  make all:       compiles all code without running
#       generator: create Halide generator
#       design:    create cpu design
#       image:     create an image with random data
#       run:       run cpu design with image
#       compare:   compare two output images
#       test:      run and compare to cpu output
#       eval:      evaluate runtime
#       clean:     remove bin directory
