HALIDE_BIN_PATH ?= ../../../..
HALIDE_SRC_PATH ?= $(HALIDE_BIN_PATH)
HALIDE_DISTRIB_PATH ?= $(HALIDE_BIN_PATH)/distrib
BIN ?= ./bin
IMAGES ?= $(HALIDE_BIN_PATH)/apps/hardware_benchmarks/images
LDFLAGS ?=
COREIR_DIR ?= $(HALIDE_BIN_PATH)/../coreir
FUNCBUF_DIR ?= $(HALIDE_BIN_PATH)/../BufferMapping/cfunc

include $(HALIDE_BIN_PATH)/apps/support/Makefile.inc

# For CoreIR generation
COREIR_CXX_FLAGS = -I $(COREIR_DIR)/include -L$(COREIR_DIR)/lib -Wl,-rpath,$(COREIR_DIR)/lib
COREIR_CXX_FLAGS += -I $(FUNCBUF_DIR)/include -L$(FUNCBUF_DIR)/bin -Wl,-rpath,$(FUNCBUF_DIR)/bin
CXXFLAGS += $(COREIR_CXX_FLAGS)
