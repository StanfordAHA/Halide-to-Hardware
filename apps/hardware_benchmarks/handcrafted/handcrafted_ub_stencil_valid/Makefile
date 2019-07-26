include ../../hw_support/Makefile.inc

TESTNAME = handcrafted_ub_stencil_valid
USE_COREIR_VALID ?= 1

include ../../hw_support/hardware_targets.mk

# these are already defined, and don't need to be rebuilt
$(BIN)/design_top.json:
$(BIN)/output_cpu.png:

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
