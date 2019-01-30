include ../../hw_support/Makefile.inc

TESTNAME = harris
USE_COREIR_VALID = 1

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


# include ../../hw_support/Makefile.inc
# 
# BIN ?= bin
# 
# all: $(BIN)/process
# 
# $(BIN)/harris.generator: harris_generator.cpp $(GENERATOR_DEPS)
# 	@-mkdir -p $(BIN)
# 	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)
# 
# $(BIN)/harris.a: $(BIN)/harris.generator
# 	@-mkdir -p $(BIN)
# 	$^ -g harris -o $(BIN) -f harris target=$(HL_TARGET)
# 
# $(BIN)/process: process.cpp $(BIN)/harris.a 
# 	@-mkdir -p $(BIN)
# 	$(CXX) $(CXXFLAGS) -I$(BIN) -Wall -O3 $^ -o $@ $(LDFLAGS)
# 
# run: $(BIN)/process
# 	@-mkdir -p $(BIN)
# 	$(BIN)/process
# 
# clean:
# 	rm -rf $(BIN)
# 
# test: run
