PNG_LIB = $(shell libpng-config --cflags --ldflags)

COREIR_DIR = ~/CppWorkspace/coreir
COREIR_CXX_FLAGS = -I$(COREIR_DIR)/include
all:
	g++ test_main.cpp -g $(COREIR_CXX_FLAGS) -I ../../../../tools/ -I ../../../../include -L ../../../../bin -lHalide $(PNG_LIB) -ljpeg -lpthread -ldl -o all-tests -std=c++11
