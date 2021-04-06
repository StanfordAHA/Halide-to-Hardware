include ../../hw_support/Makefile.inc

TESTNAME = resnet_pond
USE_COREIR_VALID = 1

include ../../hw_support/hardware_targets.mk

test-mem mem:
	@echo "Manual mapped application"

bin/output_cpu.raw:
	$(HWSUPPORT)/steveconvert.csh bin/output_cpu.png bin/output_cpu.raw

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
