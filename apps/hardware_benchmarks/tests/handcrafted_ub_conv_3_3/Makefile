include ../../hw_support/Makefile.inc

TESTNAME = handcrafted_conv_3_3
USE_COREIR_VALID ?= 1

include ../../hw_support/hardware_targets.mk

.PHONY: graph.png
graph.png: design_top.txt
	dot -Tpng $(BIN)/design_top.txt > $(BIN)/graph.png

design_top.txt: $(HWSUPPORT)/$(BIN)/coreir_to_dot
	$(HWSUPPORT)/$(BIN)/coreir_to_dot $(BIN)/design_top.json $(BIN)/design_top.txt

clean:
	mv -t . bin/design_top.json bin/output_cpu.png
	rm bin/*
	mv -t bin design_top.json output_cpu.png

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
