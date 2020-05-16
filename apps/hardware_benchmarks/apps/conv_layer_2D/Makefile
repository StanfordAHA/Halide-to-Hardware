#### Halide flags
#HALIDE_BIN_PATH := ../../../..
#HALIDE_SRC_PATH := ../../../..
include ../../hw_support/Makefile.inc

TESTNAME = conv_layer_2D
USE_COREIR_VALID = 1

include ../../hw_support/hardware_targets.mk

#all: $(BIN)/process
#
#$(BIN)/conv_layer.generator: conv_layer_generator.cpp $(GENERATOR_DEPS)
#	@-mkdir -p $(BIN)
#	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)
#
#$(BIN)/conv_layer.a: $(BIN)/conv_layer.generator
#	@-mkdir -p $(BIN)
#	$^ -g conv_layer -o $(BIN) -f conv_layer target=$(HL_TARGET) auto_schedule=false
#
#$(BIN)/conv_layer_auto_schedule.a: $(BIN)/conv_layer.generator
#	@-mkdir -p $(BIN)
#	$^ -g conv_layer -o $(BIN) -f conv_layer_auto_schedule target=$(HL_TARGET)-no_runtime auto_schedule=true
#
##$(BIN)/process: process.cpp $(BIN)/conv_layer.a $(BIN)/conv_layer_auto_schedule.a
#$(BIN)/process: process.cpp $(BIN)/conv_layer.a 
#	@-mkdir -p $(BIN)
#	$(CXX) $(CXXFLAGS) -I$(BIN) -Wall -O3 $^ -o $@ $(LDFLAGS)
#
#run: $(BIN)/process
#	@-mkdir -p $(BIN)
#	$(BIN)/process
#
#
#test: run
