PNG_LIB = $(shell libpng-config --cflags --ldflags)

all:
	g++ test_main.cpp -g -I ../../../../tools/ -I ../../../../include -L ../../../../bin -lHalide $(PNG_LIB) -ljpeg -lpthread -ldl -o all-tests -std=c++11
