HALIDE_BIN_PATH ?= ../../../../..
HALIDE_SRC_PATH ?= ../../../../..
HALIDE_DISTRIB_PATH ?= ../../../../../distrib
HWSUPPORT ?= ../../../hw_support
BIN ?= ./bin
IMAGES ?= ../../../../images
LDFLAGS ?=
COREIR_DIR ?= ../../../../../../coreir

include ../../../../support/Makefile.inc

# For CoreIR generation
COREIR_CXX_FLAGS = -I $(COREIR_DIR)/include -L$(COREIR_DIR)/lib -Wl,-rpath,$(COREIR_DIR)/lib
CXXFLAGS += $(COREIR_CXX_FLAGS)

TESTNAME = conv_2_2
USE_COREIR_VALID ?= 1

INPUT = 

include ../../../hw_support/hardware_targets.mk

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
