include ../../hw_support/Makefile.inc

TESTNAME = pond_and_mem
USE_COREIR_VALID = 1
HWSUPPORT ?= $(abspath ../../hw_support)


test-mem mem:
	@echo "Manual mapped application"

bin/output_cpu.raw:
	$(HWSUPPORT)/steveconvert.csh bin/output_cpu.png bin/output_cpu.raw

bin/input.raw:
	$(HWSUPPORT)/steveconvert.csh input.png bin/input.raw

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
