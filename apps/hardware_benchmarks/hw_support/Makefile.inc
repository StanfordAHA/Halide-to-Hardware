HALIDE_BIN_PATH ?= ../../../..
HALIDE_SRC_PATH ?= ../../../..
HALIDE_DISTRIB_PATH ?= ../../../../distrib
BIN ?= ./bin
IMAGES ?= ../../../images
LDFLAGS ?=
COREIR_DIR ?= ../../../../../coreir

include ../../../support/Makefile.inc

# For CoreIR generation
#-D_GLIBCXX_USE_CXX11_ABI=0
COREIR_CXX_FLAGS = -I $(COREIR_DIR)/include -L$(COREIR_DIR)/lib -Wl,-rpath,$(COREIR_DIR)/lib 
CXXFLAGS += $(COREIR_CXX_FLAGS)
