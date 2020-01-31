PNG_LIB = $(shell libpng-config --cflags --ldflags)

THIS_MAKEFILE = $(realpath $(filter %Makefile, $(MAKEFILE_LIST)))
ROOT_DIR = $(strip $(shell dirname $(THIS_MAKEFILE)))
COREIR_DIR ?= $(ROOT_DIR)/../../../../../coreir
COREIR_CXX_FLAGS = -I$(COREIR_DIR)/include
COREIR_LD_FLAGS = -L$(COREIR_DIR)/lib -Wl,-rpath,$(COREIR_DIR)/lib -lcoreir-commonlib -lcoreir -lcoreirsim -lcoreir-float -lfuncubuf
FUNCBUF_DIR ?= $(ROOT_DIR)/../../../../../BufferMapping/cfunc
COREIR_CXX_FLAGS += -I$(FUNCBUF_DIR)/include -L$(FUNCBUF_DIR)/bin -Wl,-rpath,$(FUNCBUF_DIR)/bin

all:
	g++ test_main.cpp ubuffer_tests.cpp -g $(COREIR_CXX_FLAGS) $(COREIR_LD_FLAGS)  -I ../../../../tools/ -I ../../../../include -L ../../../../bin -lHalide $(PNG_LIB) -ljpeg -lpthread -ldl -o all-tests -std=c++17
