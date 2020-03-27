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
FUNCUBUF_PATH ?= $(ROOT_DIR)/../../../..
HALIDE_SRC_PATH ?= ../../../..
LDFLAGS += -lcoreir-lakelib -DVERBOSE=0

# set default to TESTNAME which forces failure
TESTNAME ?= undefined_testname
TESTGENNAME ?= $(TESTNAME)
USE_COREIR_VALID ?= 0

# set this to "1>/dev/null" or "&>/dev/null" to suppress debug output to std::cout
HALIDE_DEBUG_REDIRECT ?=

HLS_PROCESS_CXX_FLAGS = -DC_TEST -Wno-unknown-pragmas -Wno-unused-label -Wno-uninitialized -Wno-literal-suffix

THIS_MAKEFILE = $(realpath $(filter %Makefile, $(MAKEFILE_LIST)))
ROOT_DIR = $(strip $(shell dirname $(THIS_MAKEFILE)))

default: all
all: $(BIN)/process

halide compiler:
	$(MAKE) -C $(HALIDE_SRC_PATH) quick_distrib
distrib:
	$(MAKE) -C $(HALIDE_SRC_PATH) distrib

$(HWSUPPORT)/$(BIN)/hardware_process_helper.o: $(HWSUPPORT)/hardware_process_helper.cpp $(HWSUPPORT)/hardware_process_helper.h
	@-mkdir -p $(HWSUPPORT)/$(BIN)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(HWSUPPORT)/$(BIN)/coreir_interpret.o: $(HWSUPPORT)/coreir_interpret.cpp $(HWSUPPORT)/coreir_interpret.h
	@-mkdir -p $(HWSUPPORT)/$(BIN)
	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -c $< -o $@ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -c $< -o $@ $(LDFLAGS) 

$(HWSUPPORT)/$(BIN)/coreir_sim_plugins.o: $(HWSUPPORT)/coreir_sim_plugins.cpp $(HWSUPPORT)/coreir_sim_plugins.h
	@-mkdir -p $(HWSUPPORT)/$(BIN)
	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -c $< -o $@ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -c $< -o $@ $(LDFLAGS)


.PHONY: generator
generator $(BIN)/$(TESTNAME).generator: $(TESTNAME)_generator.cpp $(GENERATOR_DEPS)
	@-mkdir -p $(BIN)
	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)
ifeq ($(UNAME), Darwin)
	install_name_tool -change bin/libcoreir-lakelib.so $(FUNCBUF_DIR)/bin/libcoreir-lakelib.so $@
endif

design design-cpu $(BIN)/$(TESTNAME).a: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTGENNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) $(HALIDE_DEBUG_REDIRECT)

design-coreir $(BIN)/design_top.json: $(BIN)/$(TESTNAME).generator
	#$(MAKE) $(BIN)/$(TESTNAME).generator
	@if [ $(USE_COREIR_VALID) -ne "0" ]; then \
	 make design-coreir-valid; \
	else \
	 make design-coreir-no_valid; \
	fi

coreir_to_dot $(HWSUPPORT)/$(BIN)/coreir_to_dot: $(HWSUPPORT)/coreir_to_dot.cpp $(HWSUPPORT)/coreir_to_dot.h
	@-mkdir -p $(HWSUPPORT)/$(BIN)
	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -c $< -o $@ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -o $(HWSUPPORT)/$(BIN)/coreir_to_dot $< $(LDFLAGS)

$(BIN)/design_top.txt: $(BIN)/design_top.json $(HWSUPPORT)/$(BIN)/coreir_to_dot
	$(HWSUPPORT)/$(BIN)/coreir_to_dot $(BIN)/design_top.json $(BIN)/design_top.txt

design-coreir-no_valid: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTGENNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-coreir -e coreir $(HALIDE_DEBUG_REDIRECT)

design-coreir-valid design-coreir_valid: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTGENNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-coreir-coreir_valid-use_extract_hw_kernel -e coreir $(HALIDE_DEBUG_REDIRECT)

design-verilog $(BIN)/top.v: $(BIN)/design_top.json
	@-mkdir -p $(BIN)
	./$(COREIR_DIR)/bin/coreir -i $(ROOT_DIR)/$(BIN)/design_top.json -o $(ROOT_DIR)/$(BIN)/top.v --load_libs $(COREIR_DIR)/lib/libcoreir-commonlib.so
	@echo -e "\033[0;32m coreir verilog generated \033[0m"

design-vhls $(BIN)/vhls_target.cpp $(BIN)/$(TESTNAME)_vhls.cpp: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTGENNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-hls-legacy_buffer_wrappers -e vhls $(HALIDE_DEBUG_REDIRECT)

$(BIN)/process: process.cpp $(BIN)/$(TESTNAME).a $(HWSUPPORT)/$(BIN)/hardware_process_helper.o $(HWSUPPORT)/$(BIN)/coreir_interpret.o $(HWSUPPORT)/coreir_sim_plugins.o
	@-mkdir -p $(BIN)
	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -I$(BIN) -I$(HWSUPPORT) -I$(HWSUPPORT)/xilinx_hls_lib_2015_4 -Wall $(HLS_PROCESS_CXX_FLAGS)  -O3 $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS)
	$(CXX) $(CXXFLAGS) -I$(BIN) -I$(HWSUPPORT) -I$(HWSUPPORT)/xilinx_hls_lib_2015_4 -Wall $(HLS_PROCESS_CXX_FLAGS)  -O3 $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS)
ifeq ($(UNAME), Darwin)
	install_name_tool -change bin/libcoreir-lakelib.so $(FUNCBUF_DIR)/bin/libcoreir-lakelib.so $@
endif

image image-cpu: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process image

image-ascending: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process image 1

image-float: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process image 3


$(BIN)/input.png: input.png
	@-mkdir -p $(BIN)
	cp input.png $(BIN)/input.png

$(BIN)/input.raw: input.png
	@-mkdir -p $(BIN)
	$(HWSUPPORT)/steveconvert.csh input.png $(BIN)/input.raw

$(BIN)/input.pgm: input.png
	@-mkdir -p $(BIN)
	$(eval BITWIDTH := $(shell file input.png | grep -oP "\d+-bit" | grep -oP "\d+"))
	$(eval CHANNELS := $(shell file input.png | grep -oP "\d+-bit.*? " | grep -oP "/.* "))
	if [ "$(CHANNELS)" == "" ]; then \
	  convert input.png -depth $(BITWIDTH) pgm:$(BIN)/input.pgm; \
  else \
	  convert input.png -depth $(BITWIDTH) ppm:$(BIN)/input.pgm;\
  fi

$(BIN)/%.raw: $(BIN)/%.png
	$(HWSUPPORT)/steveconvert.csh $(BIN)/$*.png $(BIN)/$*.raw

$(BIN)/%.pgm: $(BIN)/%.png
	$(eval BITWIDTH := $(shell file $(BIN)/$*.png | grep -oP "\d+-bit" | grep -oP "\d+"))
	$(eval CHANNELS := $(shell file $(BIN)/$*.png | grep -oP "\d+-bit.*? " | grep -oP "/.* "))
	if [ "$(CHANNELS)" == "" ]; then \
	  convert $(BIN)/$*.png -depth $(BITWIDTH) pgm:$(BIN)/$*.pgm; \
  else \
	  convert $(BIN)/$*.png -depth $(BITWIDTH) ppm:$(BIN)/$*.pgm;\
  fi

run run-cpu $(BIN)/output_cpu.png:
	$(MAKE) $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process run cpu input.png $(HALIDE_DEBUG_REDIRECT)

run-coreir $(BIN)/output_coreir.png: $(BIN)/process $(BIN)/design_top.json
	@-mkdir -p $(BIN)
	$(BIN)/process run coreir input.png $(HALIDE_DEBUG_REDIRECT)

run-verilog: $(BIN)/top.v $(BIN)/input.raw
	@-mkdir -p $(BIN)
	verilator --cc $(BIN)/top.v --exe $(COREIR_DIR)/tools/verilator/tb.cpp
	cd obj_dir && \
	make -f Vtop.mk CXXFLAGS="-std=c++11 -Wall -fPIC -I/nobackup/setter/h2h/coreir/include" CXX=g++-4.9 LINK=g++-4.9 && \
	./Vtop -i ../$(BIN)/input.raw -o ../$(BIN)/output_verilog.raw

run-vhls: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process run vhls input.png $(HALIDE_DEBUG_REDIRECT)

compare compare-cpu-coreir compare-coreir-cpu $(BIN)/output.png: $(BIN)/output_coreir.png $(BIN)/output_cpu.png $(BIN)/process
	$(BIN)/process compare $(BIN)/output_cpu.png $(BIN)/output_coreir.png; \
	EXIT_CODE=$$?; \
  echo $$EXIT_CODE; \
	if [[ $$EXIT_CODE = 0 ]]; then \
    cp $(BIN)/output_coreir.png $(BIN)/output.png; \
    (exit $$EXIT_CODE); \
	else \
    (exit $$EXIT_CODE);  \
	fi


eval eval-cpu: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process eval cpu input.png

eval-coreir: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process eval coreir input.png

update_golden updategolden golden: $(BIN)/output_cpu.png
	@-mkdir -p $(GOLDEN)
	cp $(BIN)/output_cpu.png $(GOLDEN)/golden_output.png

check:
	@printf "%-25s" $(TESTNAME);
	@if [ -f "$(BIN)/ubuffers.json" ]; then \
	  printf "  \033[0;32m%s\033[0m" " rewrite"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!rewrite"; \
	fi
	@if [ -f "$(BIN)/design_prepass.json" ]; then \
	  printf "  \033[0;32m%s\033[0m" " coreir"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!coreir"; \
	fi
	@if [ -f "$(BIN)/output.png" ]; then \
	  printf "  \033[0;32m%s\033[0m" " output.png"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!output.png"; \
	fi
	@#@if [ -f "passed.md5" ]; then \
	#  printf "  \033[0;32m%s\033[0m" "passed.md5"; \
	#fi
	@#@if [ -f "failed.md5" ]; then \
	#  printf "  \033[0;31m%s\033[0m" "failed.md5"; \
	#fi
	@printf "\n"

$(BIN)/graph.png: $(BIN)/design_top.txt
	dot -Tpng $(BIN)/design_top.txt > $(BIN)/graph.png
graph.png graph:
	$(MAKE) $(BIN)/graph.png

clean:
	rm -rf $(BIN)

test: run
