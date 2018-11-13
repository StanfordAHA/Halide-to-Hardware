# Usage:
#  make all:       compiles all code
#       generator: create Halide generator
#       design:    create cpu design
#       run:       run cpu design with image
#       compare:   compare two output images
#       eval:      evaluate runtime
#       golden:    copy design and output image
#       clean:     remove bin directory


# define defulats to environment variables
SHELL = bash
BIN ?= bin
GOLDEN ?= golden
HWSUPPORT ?= ../../hw_support
HALIDE_SRC_PATH ?= ../../../..

# set default to TESTNAME which forces failure
TESTNAME ?= undefined_testname

default: all
all: $(BIN)/process

halide compiler:
	$(MAKE) -C $(HALIDE_SRC_PATH) distrib

$(HWSUPPORT)/$(BIN)/hardware_process_helper.o: $(HWSUPPORT)/hardware_process_helper.cpp
	@-mkdir -p $(HWSUPPORT)/$(BIN)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: generator
generator $(BIN)/$(TESTNAME).generator: $(TESTNAME)_generator.cpp $(GENERATOR_DEPS)
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

design $(BIN)/$(TESTNAME).a: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)

design-cpu: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)

design-coreir: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-coreir -e coreir

design-hls $(BIN)/vhls_target.cpp $(BIN)/$(TESTNAME)_vhls.cpp: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-hls-legacy_buffer_wrappers -e vhls

HLS_PROCESS_CXX_FLAGS = -DC_TEST -Wno-unknown-pragmas -Wno-unused-label -Wno-uninitialized -Wno-literal-suffix
$(BIN)/process: process.cpp $(BIN)/$(TESTNAME).a $(BIN)/vhls_target.cpp $(BIN)/$(TESTNAME)_vhls.cpp $(HWSUPPORT)/$(BIN)/hardware_process_helper.o
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -I$(BIN) -I$(HWSUPPORT) -I$(HWSUPPORT)/xilinx_hls_lib_2015_4 -Wall $(HLS_PROCESS_CXX_FLAGS)  -O3 $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS)

image image-cpu: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process image

$(BIN)/output.png run run-cpu: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process run cpu input.png

run-coreir: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process run coreir input.png

run-hls: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process run hls input.png

eval eval-cpu: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process eval cpu input.png

eval-coreir: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process eval coreir input.png

update_golden updategolden golden: $(BIN)/output.png
	@-mkdir -p $(GOLDEN)
	cp $(BIN)/output.png $(GOLDEN)/golden_output.png

check:
	@printf "%-15s" $(TESTNAME);
	@if [ -f "$(BIN)/design_prepass.json" ]; then \
	  printf "  \033[0;32m%s\033[0m" " coreir"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!coreir"; \
	fi
	@if [ -f "$(BIN)/process" ]; then \
	  printf "  \033[0;32m%s\033[0m" " process"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!process"; \
	fi
	@if [ -f "$(BIN)/output.png" ]; then \
	  printf "  \033[0;32m%s\033[0m" " output.png"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!output.png"; \
	fi
	@if [ -f "passed.md5" ]; then \
	  printf "  \033[0;32m%s\033[0m" "passed.md5"; \
	fi
	@if [ -f "failed.md5" ]; then \
	  printf "  \033[0;31m%s\033[0m" "failed.md5"; \
	fi
	@printf "\n"



clean:
	rm -rf $(BIN)

test: run
