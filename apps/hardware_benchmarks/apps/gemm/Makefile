#### Halide flags
HALIDE_BIN_PATH := ../../../..
HALIDE_SRC_PATH := ../../../..
include ../../../support/Makefile.inc

BIN ?= bin

TESTNAME = gemm
USE_COREIR_VALID ?= 1

include ../../hw_support/hardware_targets.mk

all: $(BIN)/process

#$(BIN)/gemm.generator: gemm_generator.cpp $(GENERATOR_DEPS)
#	@-mkdir -p $(BIN)
#	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)
#
#$(BIN)/gemm.a: $(BIN)/gemm.generator
#	@-mkdir -p $(BIN)
#	$^ -g gemm -o $(BIN) -f gemm target=$(HL_TARGET) auto_schedule=false
#
#$(BIN)/gemm_auto_schedule.a: $(BIN)/gemm.generator
#	@-mkdir -p $(BIN)
#	$^ -g gemm -o $(BIN) -f gemm_auto_schedule target=$(HL_TARGET)-no_runtime auto_schedule=true
#
##$(BIN)/process: process.cpp $(BIN)/gemm.a $(BIN)/gemm_auto_schedule.a
#$(BIN)/process: process.cpp $(BIN)/gemm.a 
#	@-mkdir -p $(BIN)
#	$(CXX) $(CXXFLAGS) -I$(BIN) -Wall -O3 $^ -o $@ $(LDFLAGS)
#
#run: $(BIN)/process
#	@-mkdir -p $(BIN)
#	$(BIN)/process
#
#test: run
