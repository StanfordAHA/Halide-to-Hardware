HALIDE_BIN_PATH ?= ../../../..
HALIDE_SRC_PATH ?= ../../../..
HALIDE_DISTRIB_PATH ?= ../../../../distrib
BIN ?= ./bin
IMAGES ?= ../../../images
LDFLAGS ?=
COREIR_DIR ?= ../../../../../coreir
FUNCBUF_DIR ?= ../../../../../BufferMapping/cfunc

include ../../../support/Makefile.inc

# For CoreIR generation
COREIR_CXX_FLAGS = -I $(COREIR_DIR)/include -L$(COREIR_DIR)/lib -Wl,-rpath,$(COREIR_DIR)/lib
COREIR_CXX_FLAGS += -I $(FUNCBUF_DIR)/include -L$(FUNCBUF_DIR)/bin -lfuncubuf -Wl,-rpath,$(FUNCBUF_DIR)/bin
CXXFLAGS += $(COREIR_CXX_FLAGS)
