include ../../hw_support/Makefile.inc

TESTNAME = random_pipeline
USE_COREIR_VALID = 1
SEED ?= 1

include ../../hw_support/hardware_targets.mk

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

design-coreir-valid design-coreir_valid: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-coreir-coreir_valid -e coreir seed=$(SEED)
