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
FUNCUBUF_PATH ?= $(abspath $(ROOT_DIR)/../../../..)
LAKE_PATH ?= $(abspath $(CLOCKWORK_DIR)/../lake)
LDFLAGS += -lcoreir-lakelib

#WITH_CLOCKWORK ?= 0
CLOCKWORK_PATH ?= $(CLOCKWORK_DIR)
ISL_PATH ?= $(CLOCKWORK_PATH)/barvinok-0.41/isl
CLOCKWORK_CXX_FLAGS = -std=c++17 -I$(CLOCKWORK_PATH) -I$(CLOCKWORK_PATH)/include -I$(ISL_PATH) -fPIC
CLOCKWORK_LD_FLAGS = -L$(CLOCKWORK_PATH)/lib -L$(ISL_PATH) -Wl,-rpath,$(CLOCKWORK_PATH)/lib
CLOCKWORK_LD_FLAGS += -lclkwrk -lbarvinok -lisl -lntl -lgmp -lpolylibgmp -lpthread -lcoreir-float -lcoreir-float_DW

# set default to TESTNAME which forces failure
TESTNAME ?= undefined_testname
TESTGENNAME ?= $(TESTNAME)
TESTORAPP = $(shell basename $(abspath $(ROOT_DIR)/..))
USE_COREIR_VALID ?= 0
EXT ?= png
PIPELINED ?= 0
META_TARGET ?= app

# set this for Halide generator arguments
HALIDE_GEN_ARGS ?= 
HALIDE_GEN_SIZE_ARGS ?= 

# =========================== RDAI Configuration  ===================================

RDAI_HOST_CXXFLAGS 			= -I$(RDAI_DIR)/host_runtimes/$(RDAI_HOST_RUNTIME)/include
RDAI_PLATFORM_CXXFLAGS 		= -I$(RDAI_DIR)/platform_runtimes/$(RDAI_PLATFORM_RUNTIME)/include

RDAI_HOST_SRC				= $(wildcard $(RDAI_DIR)/host_runtimes/$(RDAI_HOST_RUNTIME)/src/*.cpp)
RDAI_PLATFORM_SRC			= $(wildcard $(RDAI_DIR)/platform_runtimes/$(RDAI_PLATFORM_RUNTIME)/src/*.cpp)

RDAI_HOST_SRC_FILES			= $(notdir $(RDAI_HOST_SRC))
RDAI_PLATFORM_SRC_FILES 	= $(notdir $(RDAI_PLATFORM_SRC))

RDAI_HOST_OBJ_NAMES			= $(patsubst %.cpp,%.o,$(RDAI_HOST_SRC_FILES))
RDAI_PLATFORM_OBJ_NAMES 	= $(patsubst %.cpp,%.o,$(RDAI_PLATFORM_SRC_FILES))

RDAI_HOST_OBJ_DEPS 			= $(foreach obj,$(RDAI_HOST_OBJ_NAMES),$(BIN)/rdai_host-$(obj))
RDAI_PLATFORM_OBJ_DEPS 		= $(foreach obj,$(RDAI_PLATFORM_OBJ_NAMES),$(BIN)/rdai_platform-$(obj))

# =========================== End of RDAI ======================================


# set this to "1>/dev/null" or "&>/dev/null" to suppress debug output to std::cout
HALIDE_DEBUG_REDIRECT ?=
# this is used in the buffermapping (especially ubuffer simulation)
VERBOSE ?= 0
ifeq ($(VERBOSE), 0)
	LDFLAGS += -DVERBOSE=0
else
	LDFLAGS += -DVERBOSE=1
endif


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
	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -c $< $(LDFLAGS) -o $@
	$(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -c $< $(LDFLAGS) -o $@ 

$(HWSUPPORT)/$(BIN)/coreir_sim_plugins.o: $(HWSUPPORT)/coreir_sim_plugins.cpp $(HWSUPPORT)/coreir_sim_plugins.h
	@-mkdir -p $(HWSUPPORT)/$(BIN)
	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -c $< -o $@ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -c $< $(LDFLAGS) -o $@


.PHONY: generator
generator $(BIN)/$(TESTNAME).generator: $(TESTNAME)_generator.cpp $(GENERATOR_DEPS)
	@-mkdir -p $(BIN)
	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) $(LDFLAGS) -o $@
ifeq ($(UNAME), Darwin)
	install_name_tool -change bin/libcoreir-lakelib.so $(FUNCBUF_DIR)/bin/libcoreir-lakelib.so $@
endif

design cpu design-cpu $(BIN)/$(TESTNAME).a: $(BIN)/$(TESTNAME).generator $(BIN)/halide_gen_args
	@-mkdir -p $(BIN)
	$< -g $(TESTGENNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) $(HALIDE_GEN_SIZE_ARGS) $(HALIDE_GEN_ARGS) $(HALIDE_DEBUG_REDIRECT)

coreir design-coreir: $(BIN)/$(TESTNAME).generator
	@if [ $(USE_COREIR_VALID) -ne "0" ]; then \
	 make design-coreir-valid; \
	else \
	 make design-coreir-no_valid; \
	fi

coreir_to_dot $(HWSUPPORT)/$(BIN)/coreir_to_dot: $(HWSUPPORT)/coreir_to_dot.cpp $(HWSUPPORT)/coreir_to_dot.h
	@-mkdir -p $(HWSUPPORT)/$(BIN)
	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -c $< -o $@ $(LDFLAGS) 
	@#$(CXX) $(CXXFLAGS) -I$(HWSUPPORT) $< $(LDFLAGS) -o $(HWSUPPORT)/$(BIN)/coreir_to_dot
	@#$(CXX) $(CXXFLAGS) -I$(HWSUPPORT) $(CLOCKWORK_PATH)/coreir_backend.o $< $(LDFLAGS) -Wno-sign-compare -I$(CLOCKWORK_PATH) -L$(CLOCKWORK_PATH)/lib $(CLOCKWORK_CXX_FLAGS) $(CLOCKWORK_LD_FLAGS) -lcoreir-cgralib -o $(HWSUPPORT)/$(BIN)/coreir_to_dot
	$(CXX) $(CXXFLAGS) -I$(HWSUPPORT) $< $(LDFLAGS) -Wno-sign-compare -I$(CLOCKWORK_PATH) -L$(CLOCKWORK_PATH)/lib $(CLOCKWORK_CXX_FLAGS) $(CLOCKWORK_LD_FLAGS) -lcoreir-cgralib -o $(HWSUPPORT)/$(BIN)/coreir_to_dot

coreir_tree_reduction $(HWSUPPORT)/$(BIN)/coreir_tree_reduction: $(HWSUPPORT)/coreir_tree_reduction.cpp
	@-mkdir -p $(HWSUPPORT)/$(BIN)
	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -I$(HWSUPPORT) -c $< -o $@ $(LDFLAGS) 
	@#$(CXX) $(CXXFLAGS) -I$(HWSUPPORT) $< $(LDFLAGS) -o $(HWSUPPORT)/$(BIN)/coreir_to_dot
	@#$(CXX) $(CXXFLAGS) -I$(HWSUPPORT) $(CLOCKWORK_PATH)/coreir_backend.o $< $(LDFLAGS) -Wno-sign-compare -I$(CLOCKWORK_PATH) -L$(CLOCKWORK_PATH)/lib $(CLOCKWORK_CXX_FLAGS) $(CLOCKWORK_LD_FLAGS) -lcoreir-cgralib -o $(HWSUPPORT)/$(BIN)/coreir_to_dot
	$(CXX) $(CXXFLAGS) -I$(HWSUPPORT) $< $(LDFLAGS) -Wno-sign-compare -I$(CLOCKWORK_PATH) -L$(CLOCKWORK_PATH)/lib $(CLOCKWORK_CXX_FLAGS) $(CLOCKWORK_LD_FLAGS) -lcoreir-cgralib -o $(HWSUPPORT)/$(BIN)/coreir_tree_reduction


$(BIN)/design_top_graph.json:
	cp $(BIN)/design_top.json $(BIN)/design_top_graph.json
#$(BIN)/design_top_graph.json: $(BIN)/design_top.json
#	cp $(BIN)/map_result/$(TESTNAME)/$(TESTNAME).json $(BIN)/design_top_graph.json

#$(BIN)/design_top.txt: $(BIN)/design_top.json $(HWSUPPORT)/$(BIN)/coreir_to_dot
$(BIN)/design_top.txt: $(HWSUPPORT)/$(BIN)/coreir_to_dot $(BIN)/design_top_graph.json
	cat $(BIN)/design_top_graph.json | sed "s/\([0-9]*\)\],\"Arg\",\"init\"/\1],\"\1\'h0\"/g" > $(BIN)/design_top_fixed.json
	$(HWSUPPORT)/$(BIN)/coreir_to_dot $(BIN)/design_top_fixed.json $(BIN)/design_top.txt

design-coreir-no_valid: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTGENNAME) -f $(TESTNAME) target=$(HL_TARGET)-coreir -e coreir $(HALIDE_DEBUG_REDIRECT) $(HALIDE_GEN_ARGS) -o $(BIN)

design-coreir-valid design-coreir_valid: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTGENNAME) -f $(TESTNAME) target=$(HL_TARGET)-coreir-coreir_valid-use_extract_hw_kernel -e coreir,html $(HALIDE_GEN_ARGS) $(HALIDE_DEBUG_REDIRECT) -o $(BIN)

$(BIN)/$(TESTNAME)_clockwork.cpp $(BIN)/$(TESTNAME)_clockwork.h \
$(BIN)/clockwork_testscript.h $(BIN)/clockwork_testscript.cpp $(BIN)/clockwork_codegen.cpp \
clockwork design-clockwork $(BIN)/$(TESTNAME)_memory.cpp $(BIN)/$(TESTNAME)_compute.h: $(BIN)/$(TESTNAME).generator $(BIN)/halide_gen_args
	@-mkdir -p $(BIN)
	$< -g $(TESTGENNAME) -f $(TESTNAME) target=$(HL_TARGET)-clockwork -e clockwork,html $(HALIDE_GEN_SIZE_ARGS) $(HALIDE_GEN_ARGS) $(HALIDE_DEBUG_REDIRECT) -o $(BIN)
	(echo "//" `cat $(BIN)/halide_gen_args`) | cat >> $(BIN)/$(TESTNAME)_memory.cpp


#: $(BIN)/$(TESTNAME)_memory.cpp $(BIN)/$(TESTNAME)_compute.h

ALL_UNOPT_CPPS = $(wildcard $(BIN)/unoptimized_*.cpp)
IGNORED_UNOPT_CPPS =  $(foreach file,$(ALL_UNOPT_CPPS),$(if $(findstring _host.cpp,$(file)),$(file),))
IGNORED_UNOPT_CPPS += $(foreach file,$(ALL_UNOPT_CPPS),$(if $(findstring _sw_bmp_test_harness.cpp,$(file)),$(file),))
UNOPTIMIZED_CPPS = $(filter-out $(IGNORED_UNOPT_CPPS), $(wildcard $(BIN)/unoptimized_*.cpp))
UNOPTIMIZED_OBJS = $(patsub %.cpp,%.o,$(UNOPTIMIZED_CPPS))

$(BIN)/clockwork_codegen.o: $(BIN)/clockwork_codegen.cpp
	$(CXX) $(CLOCKWORK_CXX_FLAGS) -c $< -o $@
$(BIN)/clockwork_codegen: $(BIN)/clockwork_codegen.o
	$(CXX) $(CLOCKWORK_CXX_FLAGS) $^ $(CLOCKWORK_LD_FLAGS) -L $(COREIR_DIR)/lib -Wl,-rpath $(COREIR_DIR)/lib -lcoreir -lcoreirsim -lcoreir-commonlib -lcoreir-float -lcoreir-float_DW -o $@
$(BIN)/unoptimized_$(TESTNAME).cpp unopt-clockwork clockwork-unopt unopt: $(BIN)/clockwork_codegen
	cd $(BIN) && LD_LIBRARY_PATH=$(CLOCKWORK_PATH)/lib:$(COREIR_DIR)/lib \
	./clockwork_codegen unopt 1>mem_cout 2> >(tee -a mem_cout >&2); \
	EXIT_CODE=$$?; cd ..; \exit $$EXIT_CODE
$(BIN)/optimized_$(TESTNAME).cpp opt-clockwork clockwork-opt opt: $(BIN)/clockwork_codegen
	cd $(BIN) && LD_LIBRARY_PATH=$(CLOCKWORK_PATH)/lib:$(COREIR_DIR)/lib \
	./clockwork_codegen opt 1>mem_cout 2> >(tee -a mem_cout >&2); \
	EXIT_CODE=$$?; cd ..; exit $$EXIT_CODE

compile_mem compile-mem mem-clockwork clockwork-mem $(BIN)/map_result/$(TESTNAME)/$(TESTNAME).json: $(BIN)/clockwork_codegen
#	cp /aha/MetaMapper/libs/*_header.json $(BIN)/ && cp /aha/MetaMapper/libs/*_header.json /aha/clockwork/ && cp /aha/MetaMapper/libs/*_header.json /aha/garnet/headers/
	@echo -n "preprocessing starting at:" && date
	cd $(BIN) && \
	CLKWRK_PATH=$(CLOCKWORK_PATH) LD_LIBRARY_PATH=$(CLOCKWORK_PATH)/lib:$(COREIR_DIR)/lib LAKE_PATH=$(LAKE_PATH) LAKE_CONTROLLERS=$(abspath $(BIN)) LAKE_STREAM=$(BIN) COREIR_PATH=$(COREIR_DIR) \
	./clockwork_codegen compile_mem 1>mem_cout 2> >(tee -a mem_cout >&2); \
	EXIT_CODE=$$?; rm unoptimized_$(TESTNAME)*; cd ..; exit $$EXIT_CODE
#EXIT_CODE=$$?; cd ..; exit $$EXIT_CODE

memtest test_mem test-mem test-mem-clockwork clockwork-mem-test mem-test: $(BIN)/clockwork_codegen
	@mkdir -p $(BIN)/coreir_compute && cp $(BIN)/$(TESTNAME)_compute.json $(BIN)/coreir_compute/$(TESTNAME)_compute.json
	cd $(BIN) && \
	CLKWRK_PATH=$(CLOCKWORK_PATH) LD_LIBRARY_PATH=$(CLOCKWORK_PATH)/lib:$(COREIR_DIR)/lib LAKE_PATH=$(LAKE_PATH) LAKE_CONTROLLERS=$(abspath $(BIN)) LAKE_STREAM=$(BIN) COREIR_PATH=$(COREIR_DIR) \
	./clockwork_codegen compile_and_test_mem 1>mem_cout 2> >(tee -a mem_cout >&2); \
	EXIT_CODE=$$?; cd ..; exit $$EXIT_CODE

tree: $(HWSUPPORT)/$(BIN)/coreir_tree_reduction
	cp $(BIN)/$(TESTNAME)_compute.json $(BIN)/$(TESTNAME)_compute_old.json && \
	$(HWSUPPORT)/$(BIN)/coreir_tree_reduction $(BIN)/$(TESTNAME)_compute_old.json $(BIN)/$(TESTNAME)_compute_tree.json && \
	cp $(BIN)/$(TESTNAME)_compute_tree.json $(BIN)/$(TESTNAME)_compute.json

treegraph tree_graph: tree
	$(MAKE) compile-mem && make mem && cp $(BIN)/design_top.json $(BIN)/design_top_graph.json && $(MAKE) graph.png

delay_mem:
	rm -rf bin/map_result/ && make compile-mem
	make mem

reschedule_mem:
	rm -rf bin/map_result && make compile-mem
	python $(HWSUPPORT)/copy_clockwork_schedules.py $(BIN)/map_result/$(TESTNAME)/$(TESTNAME)_to_metamapper.json $(BIN)/design_top.json $(BIN)/$(TESTNAME)_flush_latencies.json $(BIN)/$(TESTNAME)_pond_latencies.json

mem design_top design_top.json $(BIN)/design_top.json: $(BIN)/map_result/$(TESTNAME)/$(TESTNAME).json
	cp $(BIN)/map_result/$(TESTNAME)/$(TESTNAME)_garnet.json $(BIN)/design_top.json

map $(BIN)/$(TESTNAME)_compute_mapped.json: $(BIN)/$(TESTNAME)_compute.json
	make tree
	python /aha/MetaMapper/scripts/map_$(META_TARGET).py $(BIN)/$(TESTNAME)_compute.json $(PIPELINED)
	sed -i -e 's/_mapped//g' $(BIN)/$(TESTNAME)_compute_mapped.json

#FIXME: $(BIN)/unoptimized_$(TESTNAME).o
$(BIN)/clockwork_testscript.o: $(BIN)/clockwork_testscript.cpp $(UNOPTIMIZED_OBJS) $(BIN)/unoptimized_$(TESTNAME).o
	$(CXX) $(CXXFLAGS) -I$(CLOCKWORK_PATH)  -c $< -o $@
#$(BIN)/unoptimized_%.o: $(BIN)/unoptimized_%.cpp
#	@if [ ! -f "$(BIN)/unoptimized_$(TESTNAME).cpp" ]; then \
#		$(MAKE) $(UNOPTIMIZED_OBJS); \
#		ld -relocatable $(UNOPTIMIZED_OBJS) -o $@; \
#	else \
#		$(CXX) $(CXXFLAGS) -I$(CLOCKWORK_PATH)  -c $< -o $@; \
#	fi
$(BIN)/unoptimized_%.o: $(BIN)/unoptimized_%.cpp
	$(CXX) $(CXXFLAGS) -I$(CLOCKWORK_PATH)  -c $< -o $@
$(BIN)/$(TESTNAME)_clockwork.o: $(BIN)/$(TESTNAME)_clockwork.cpp $(BIN)/$(TESTNAME)_clockwork.h
	@echo -e "\n[COMPILE_INFO] building clockwork pipeline"
	$(CXX) $(CXXFLAGS) -I$(CLOCKWORK_PATH) -c $< -o $@
$(BIN)/rdai_clockwork_platform.h: $(BIN)/$(TESTNAME)_clockwork.o
$(BIN)/rdai_host-%.o: $(RDAI_DIR)/host_runtimes/$(RDAI_HOST_RUNTIME)/src/%.cpp
	@echo -e "\n[COMPILE_INFO] building RDAI host runtime"
	$(CXX) $(CXXFLAGS) -I$(CLOCKWORK_PATH) $(RDAI_HOST_CXXFLAGS) -c $^ -o $@
$(BIN)/rdai_platform-rdai_clockwork_platform.o: $(RDAI_DIR)/platform_runtimes/$(RDAI_PLATFORM_RUNTIME)/src/rdai_clockwork_platform.cpp $(BIN)/rdai_clockwork_platform.h
	@echo -e "\n[COMPILE_INFO] building RDAI platform runtime"
	$(CXX) $(CXXFLAGS) -I$(BIN) -I$(CLOCKWORK_PATH) $(RDAI_PLATFORM_CXXFLAGS) -c $(RDAI_DIR)/platform_runtimes/$(RDAI_PLATFORM_RUNTIME)/src/rdai_clockwork_platform.cpp -o $@
$(BIN)/rdai_platform-%.o: $(RDAI_DIR)/platform_runtimes/$(RDAI_PLATFORM_RUNTIME)/src/%.cpp
	@echo -e "\n[COMPILE_INFO] building RDAI platform runtime"
	$(CXX) $(CXXFLAGS) -I$(BIN) -I$(CLOCKWORK_PATH) $(RDAI_PLATFORM_CXXFLAGS) -c $^ -o $@
$(BIN)/halide_runtime.o: $(BIN)/$(TESTNAME).generator
	@echo -e "\n[COMPILE_INFO] building Halide runtime"
	$^ -r halide_runtime -e o  target=$(HL_TARGET) -o $(BIN)

#$(BIN)/process_clockwork: process.cpp \
#						  $(HWSUPPORT)/$(BIN)/hardware_process_helper.o \
#						  $(HWSUPPORT)/$(BIN)/coreir_interpret.o \
#						  $(HWSUPPORT)/$(BIN)/coreir_sim_plugins.o \
#						  $(BIN)/halide_runtime.o \
#							$(BIN)/clockwork_testscript.o \
#						  $(BIN)/unoptimized_$(TESTNAME).o \
#						  $(BIN)/$(TESTNAME)_clockwork.o \
#						  $(RDAI_HOST_OBJ_DEPS) \
#						  $(RDAI_PLATFORM_OBJ_DEPS) \
#							$(BIN)/$(TESTNAME).a
#	@echo -e "\n[COMPILE_INFO] building process_clockwork"
#	$(CXX) 	$(CXXFLAGS) -O3 -I$(BIN) -I$(HWSUPPORT) -Wall $(RDAI_PLATFORM_CXXFLAGS) $(HLS_PROCESS_CXX_FLAGS) \
#			-DWITH_CLOCKWORK $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS) -no-pie

design-verilog $(BIN)/top.v: $(BIN)/design_top.json
	@-mkdir -p $(BIN)
	./$(COREIR_DIR)/bin/coreir -i $(ROOT_DIR)/$(BIN)/design_top.json --load_libs $(COREIR_DIR)/lib/libcoreir-commonlib.so -o $(ROOT_DIR)/$(BIN)/top.v
	@echo -e "\033[0;32m coreir verilog generated \033[0m"

design-vhls $(BIN)/vhls_target.cpp $(BIN)/$(TESTNAME)_vhls.cpp: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTGENNAME) -f $(TESTNAME) target=$(HL_TARGET)-hls-legacy_buffer_wrappers -e vhls,html $(HALIDE_DEBUG_REDIRECT) -o $(BIN)

#$(BIN)/process: process.cpp $(BIN)/$(TESTNAME).a $(HWSUPPORT)/$(BIN)/hardware_process_helper.o $(HWSUPPORT)/$(BIN)/coreir_interpret.o $(HWSUPPORT)/coreir_sim_plugins.o
#	@-mkdir -p $(BIN)
#	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -I$(BIN) -I$(HWSUPPORT) -I$(HWSUPPORT)/xilinx_hls_lib_2015_4 -Wall $(HLS_PROCESS_CXX_FLAGS)  -O3 $^ $(LDFLAGS) $(IMAGE_IO_FLAGS) -o $@
#	@#$(CXX) $(CXXFLAGS) -I$(BIN) -I$(HWSUPPORT) -I$(HWSUPPORT)/xilinx_hls_lib_2015_4 -Wall $(HLS_PROCESS_CXX_FLAGS)  -O3 $^ $(LDFLAGS) $(IMAGE_IO_FLAGS) -o $@
#	$(CXX) $(CXXFLAGS) -I$(BIN) -I$(HWSUPPORT) -Wall $(HLS_PROCESS_CXX_FLAGS)  -O3 $^ $(LDFLAGS) $(IMAGE_IO_FLAGS) -o $@
#ifeq ($(UNAME), Darwin)
#	install_name_tool -change bin/libcoreir-lakelib.so $(FUNCBUF_DIR)/bin/libcoreir-lakelib.so $@
#endif

#$(HWSUPPORT)/hardware_image_helpers.h

# Note: these are all set in the first pass of the makefile
PROCESS_DEPS = process.cpp $(HWSUPPORT)/$(BIN)/hardware_process_helper.o $(HWSUPPORT)/$(BIN)/coreir_sim_plugins.o 
PROCESS_TARGETS =

# conditionally add CPU implementation to process
ifneq ("$(wildcard $(BIN)/$(TESTNAME).a)","")
  WITH_CPU = 1
endif
ifeq ($(WITH_CPU),1)
  PROCESS_DEPS += $(BIN)/$(TESTNAME).a
  PROCESS_TARGETS += -DWITH_CPU
endif

# conditionally add clockwork implementation to process
ifneq ("$(wildcard $(BIN)/unoptimized_$(TESTNAME).o)","")
  WITH_CLOCKWORK = 1
endif

ifeq ($(WITH_CLOCKWORK),1)
  PROCESS_DEPS += $(BIN)/clockwork_testscript.o \
						  $(BIN)/$(TESTNAME)_clockwork.o \
						  $(BIN)/unoptimized_$(TESTNAME).o \
						  $(RDAI_HOST_OBJ_DEPS) \
						  $(RDAI_PLATFORM_OBJ_DEPS) \
						  $(BIN)/halide_runtime.o
  PROCESS_TARGETS += -DWITH_CLOCKWORK
endif

# conditionally add coreir implementation to process
ifneq ("$(wildcard $(BIN)/design_top.json)","")
  WITH_COREIR = 1
endif
ifeq ($(WITH_COREIR),1)
  PROCESS_DEPS += $(HWSUPPORT)/$(BIN)/coreir_interpret.o 
  PROCESS_TARGETS += -DWITH_COREIR
endif

#$(BIN)/process: process.cpp \
#				$(BIN)/$(TESTNAME).a \
#				$(HWSUPPORT)/$(BIN)/hardware_process_helper.o \
#				$(HWSUPPORT)/$(BIN)/coreir_interpret.o \
#				$(HWSUPPORT)/coreir_sim_plugins.o


PROCESS_TARGETS_DEFINED="coreir=$(WITH_COREIR) cpu=$(WITH_CPU) clockwork=$(WITH_CLOCKWORK)"

# Always run this, but only write the file if the variable changes
$(BIN)/process_targets: FORCE
	@-mkdir -p $(BIN)
	@if [ ! -f "$(BIN)/process_targets" ]; then \
		touch $(BIN)/process_targets; \
	fi
	@LAST_PROCESS_TARGETS_DEFINED=`cat $(BIN)/process_targets`; \
	if [[ "$$LAST_PROCESS_TARGETS_DEFINED" == 'empty' && $(PROCESS_TARGETS_DEFINED) == '' ]]; then \
		echo "PROCESS_TARGETS_DEFINED still empty"; \
	elif [[ $(PROCESS_TARGETS_DEFINED) == '' ]]; then \
		echo "PROCESS_TARGETS_DEFINED is empty. Writing to file"; \
		echo "empty" > $@; \
	elif [[ "$$LAST_PROCESS_TARGETS_DEFINED" != $(PROCESS_TARGETS_DEFINED) ]]; then \
		echo "PROCESS_TARGETS_DEFINED changed to $(PROCESS_TARGETS_DEFINED)"; \
		echo $(PROCESS_TARGETS_DEFINED) > $@; \
	else \
		echo "PROCESS_TARGETS_DEFINED has not changed from '$(PROCESS_TARGETS_DEFINED)'"; \
	fi

# we should remake process in case there are extra dependencies
#.PHONY: $(BIN)/process
$(BIN)/process: $(PROCESS_DEPS) $(BIN)/process_targets $(HWSUPPORT)/hardware_image_helpers.h
	@#echo coreir=$(WITH_COREIR) cpu=$(WITH_CPU) clockwork=$(WITH_CLOCKWORK)
	@-mkdir -p $(BIN)
	@#env LD_LIBRARY_PATH=$(COREIR_DIR)/lib $(CXX) $(CXXFLAGS) -I$(BIN) -I$(HWSUPPORT) -I$(HWSUPPORT)/xilinx_hls_lib_2015_4 -Wall $(HLS_PROCESS_CXX_FLAGS)  -O3 $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS)
	@#$(CXX) $(CXXFLAGS) -I$(BIN) -I$(HWSUPPORT) -I$(HWSUPPORT)/xilinx_hls_lib_2015_4 -Wall $(HLS_PROCESS_CXX_FLAGS)  -O3 $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS)
	$(CXX) -I$(BIN) $(CXXFLAGS) -I$(HWSUPPORT) -Wall $(RDAI_PLATFORM_CXXFLAGS) $(HLS_PROCESS_CXX_FLAGS) -O3 $(PROCESS_DEPS) $(LDFLAGS) $(IMAGE_IO_FLAGS) -no-pie $(PROCESS_TARGETS) -o $@
ifeq ($(UNAME), Darwin)
	install_name_tool -change bin/libcoreir-lakelib.so $(FUNCBUF_DIR)/bin/libcoreir-lakelib.so $@
endif

# Always run this, but only write the file if the variable changes
$(BIN)/halide_gen_args: FORCE
	@-mkdir -p $(BIN)
	@if [ ! -f "$(BIN)/halide_gen_args" ]; then \
		touch $(BIN)/halide_gen_args; \
	fi
	@LAST_HALIDE_GEN_ARGS=`cat $(BIN)/halide_gen_args`; \
	if [[ "$$LAST_HALIDE_GEN_ARGS" == 'empty' && "$$HALIDE_GEN_ARGS" == '' ]]; then \
		echo "HALIDE_GEN_ARGS still empty"; \
	elif [[ "$$HALIDE_GEN_ARGS" == '' ]]; then \
		echo "HALIDE_GEN_ARGS is empty. Writing to file"; \
		echo "empty" > $@; \
	elif [[ "$$LAST_HALIDE_GEN_ARGS" != '$(HALIDE_GEN_ARGS)' ]]; then \
		echo "HALIDE_GEN_ARGS changed to $(HALIDE_GEN_ARGS)"; \
		echo $(HALIDE_GEN_ARGS) > $@; \
	else \
		echo "HALIDE_GEN_ARGS has not changed from '$(HALIDE_GEN_ARGS)'"; \
	fi

FORCE:

image image-cpu: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process image $(HALIDE_GEN_ARGS) 

image-ascending: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process image 1 $(HALIDE_GEN_ARGS) 

image-float: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process image 3 $(HALIDE_GEN_ARGS) 

$(BIN)/input.$(EXT): input.$(EXT)
	@-mkdir -p $(BIN)
	cp input.$(EXT) $(BIN)/input.$(EXT)

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

$(BIN)/input_nn.pgm: input_padded.mat kernel.mat
	@-mkdir -p $(BIN)
	python $(HWSUPPORT)/interleave_input.py $(BIN)/input_nn.pgm; 

$(BIN)/%.raw: $(BIN)/%.$(EXT)
	if [ "$(EXT)" == "png" ]; then \
	  $(HWSUPPORT)/steveconvert.csh $(BIN)/$*.png $(BIN)/$*.raw; \
	elif [ "$(EXT)" == "mat" ]; then \
	  python $(HWSUPPORT)/mat2raw.py $(BIN)/$*.$(EXT) $(BIN)/$*.raw; \
	else \
	  echo "Unsupported file format: $(EXT)"; \
  fi

$(BIN)/%.raw: $(BIN)/%.leraw
	dd conv=swab <$(BIN)/$*.leraw >$(BIN)/$*.raw

io inout inputs inputfiles rawio rawios ioraw ioraws : $(BIN)/design_meta_halide.json
	python $(HWSUPPORT)/generate_raw_files.py $^

.PHONY: $(BIN)/cgra_config.json
$(BIN)/cgra_config.json:
	@-mkdir -p $(BIN)
	if [ -f cgra_config.json ]; then \
		cp cgra_config.json $@; \
	else \
		cp $(HWSUPPORT)/default_config.json $@; \
	fi

$(BIN)/cgra_input.pgm $(BIN)/input_cgra.pgm: $(BIN)/cgra_config.json $(BIN)/output_clockwork.$(EXT)
	python3 $(HWSUPPORT)/interleave_cgrainput.py $(BIN)/cgra_config.json $@

$(BIN)/cgra_output.pgm $(BIN)/output_cgra.pgm $(BIN)/output_cgra_comparison.pgm $(BIN)/gold.pgm: $(BIN)/output_clockwork.$(EXT) $(BIN)/hw_output_header.txt
	$(MAKE) $(BIN)/hw_output.raw
	cat $(BIN)/hw_output_header.txt > $@
	cat $(BIN)/hw_output.raw >> $@

# create the output file, but convert it to 8bits by dropping every other byte
$(BIN)/output_cgra_comparison8.pgm: $(BIN)/output_clockwork.$(EXT) $(BIN)/hw_output_header.txt
	$(MAKE) $(BIN)/hw_output.raw
	xxd -p bin/hw_output.raw | sed -E 's/..(..)/\1/g' | xxd -r -p > bin/hw_output8.raw
	cat $(BIN)/hw_output_header.txt | sed 's/65535/255/g' | cat > $@
	cat $(BIN)/hw_output8.raw >> $@


$(BIN)/%.pgm: $(BIN)/%.png
	$(eval BITWIDTH := $(shell file $(BIN)/$*.png | grep -oP "\d+-bit" | grep -oP "\d+"))
	$(eval CHANNELS := $(shell file $(BIN)/$*.png | grep -oP "\d+-bit.*? " | grep -oP "/.* "))
	if [ "$(CHANNELS)" == "" ]; then \
	  convert $(BIN)/$*.png -depth $(BITWIDTH) pgm:$(BIN)/$*.pgm; \
  else \
	  convert $(BIN)/$*.png -depth $(BITWIDTH) ppm:$(BIN)/$*.pgm;\
  fi

#$(BIN)/output_cpu.pgm : $(BIN)/output_cpu.mat
#	@-mkdir -p $(BIN)
#	python $(HWSUPPORT)/mat2pgm.py $(BIN)/output_cpu.mat $(BIN)/output_cpu.pgm; 

run run-cpu $(BIN)/output_cpu.$(EXT): $(BIN)/$(TESTNAME).a $(BIN)/process
	@-mkdir -p $(BIN)
	$(MAKE) $(BIN)/process WITH_CPU=1
	$(HALIDE_GEN_ARGS) EXT=$(EXT) $(BIN)/process run cpu input.png $(HALIDE_DEBUG_REDIRECT)

run-coreir $(BIN)/output_coreir.$(EXT): $(BIN)/design_top.json
	@-mkdir -p $(BIN)
	$(MAKE) $(BIN)/process WITH_COREIR=1
	$(HALIDE_GEN_ARGS) EXT=$(EXT) $(BIN)/process run coreir input.png $(HALIDE_DEBUG_REDIRECT)

run-rewrite $(BIN)/output_rewrite.png: $(BIN)/design_top.json
	@-mkdir -p $(BIN)
	$(MAKE) $(BIN)/process WITH_COREIR=1
	$(HALIDE_GEN_ARGS) EXT=$(EXT) $(BIN)/process run rewrite input.png $(HALIDE_DEBUG_REDIRECT)

run-clockwork $(BIN)/output_clockwork.$(EXT) $(BIN)/design_meta_halide.json: $(BIN)/process $(BIN)/clockwork_testscript.o
	@-mkdir -p $(BIN)
	$(MAKE) $(BIN)/process WITH_CLOCKWORK=1
	$(HALIDE_GEN_ARGS) EXT=$(EXT) $(BIN)/process run clockwork input.png $(HALIDE_DEBUG_REDIRECT)

run-verilog: $(BIN)/top.v $(BIN)/input.raw
	@-mkdir -p $(BIN)
	verilator --cc $(BIN)/top.v --exe $(COREIR_DIR)/tools/verilator/tb.cpp
	cd obj_dir && \
	make -f Vtop.mk CXXFLAGS="-std=c++11 -Wall -fPIC -I/nobackup/setter/h2h/coreir/include" CXX=g++-4.9 LINK=g++-4.9 && \
	./Vtop -i ../$(BIN)/input.raw -o ../$(BIN)/output_verilog.raw

run-vhls: $(BIN)/process
	@-mkdir -p $(BIN)
	$(HALIDE_GEN_ARGS) EXT=$(EXT) $(BIN)/process run vhls input.png $(HALIDE_DEBUG_REDIRECT)

#compare compare-coreir compare-cpu-coreir compare-coreir-cpu output.png $(BIN)/output.png: $(BIN)/output_coreir.png $(BIN)/output_cpu.png
compare-coreir compare-cpu-coreir compare-coreir-cpu $(BIN)/output.$(EXT):
	$(MAKE) $(BIN)/output_cpu.$(EXT)
	$(MAKE) $(BIN)/output_coreir.$(EXT)
	$(HALIDE_GEN_ARGS) $(BIN)/process compare $(BIN)/output_cpu.$(EXT) $(BIN)/output_coreir.$(EXT); \
	EXIT_CODE=$$?; \
	echo $$EXIT_CODE; \
	if [[ $$EXIT_CODE = 0 ]]; then \
    cp $(BIN)/output_coreir.$(EXT) $(BIN)/output.$(EXT); \
    (exit $$EXIT_CODE); \
	else \
    (exit $$EXIT_CODE);  \
	fi

#compare-rewrite compare-rewrite-cpu compare-cpu-rewrite: $(BIN)/output_rewrite.png $(BIN)/output_cpu.png $(BIN)/process
compare-rewrite compare-rewrite-cpu compare-cpu-rewrite:
	$(MAKE) $(BIN)/output_cpu.$(EXT)
	$(MAKE) $(BIN)/output_rewrite.$(EXT)
	$(HALIDE_GEN_ARGS) $(BIN)/process compare $(BIN)/output_cpu.$(EXT) $(BIN)/output_rewrite.$(EXT); \
	EXIT_CODE=$$?; \
	echo $$EXIT_CODE; \
	if [[ $$EXIT_CODE = 0 ]]; then \
    cp $(BIN)/output_rewrite.$(EXT) $(BIN)/output.$(EXT); \
    (exit $$EXIT_CODE); \
	else \
    (exit $$EXIT_CODE);  \
	fi

compare compare-clockwork compare-cpu-clockwork compare-clockwork-cpu output.$(EXT): $(BIN)/output_clockwork.$(EXT) $(BIN)/output_cpu.$(EXT) $(BIN)/process
#compare-clockwork compare-cpu-clockwork compare-clockwork-cpu:
#	$(MAKE) $(BIN)/output_cpu.$(EXT)
#	$(MAKE) $(BIN)/output_clockwork.$(EXT) 
	$(HALIDE_GEN_ARGS) $(BIN)/process compare $(BIN)/output_cpu.$(EXT) $(BIN)/output_clockwork.$(EXT); \
	EXIT_CODE=$$?; \
	echo $$EXIT_CODE; \
	if [[ $$EXIT_CODE = 0 ]]; then \
    cp $(BIN)/output_clockwork.$(EXT) $(BIN)/output.$(EXT); \
    (exit $$EXIT_CODE); \
	else \
    (exit $$EXIT_CODE);  \
	fi

#ahahalide aha_halide aha-halide:
#	$(MAKE) compare && \
#	$(MAKE) bin/input_cgra.pgm --no-print-directory && \
#	$(MAKE) bin/output_cgra.pgm --no-print-directory
ahahalide aha_halide aha-halide:
	$(MAKE) compare && \
	$(MAKE) rawio

ahahalidemem aha_halide_mem aha-halide-mem:
	$(MAKE) compare && \
	$(MAKE) bin/input_cgra.pgm --no-print-directory && \
	$(MAKE) bin/output_cgra.pgm --no-print-directory && \
	$(MAKE) mem

# Do all evaluation: 
eval: $(BIN)/process
	$(HALIDE_GEN_ARGS) $(BIN)/process eval cpu input.$(EXT)

eval-cpu: $(BIN)/process
	@-mkdir -p $(BIN)
	$(HALIDE_GEN_ARGS) $(BIN)/process eval cpu input.$(EXT)

eval-coreir: $(BIN)/process
	@-mkdir -p $(BIN)
	$(HALIDE_GEN_ARGS) $(BIN)/process eval coreir input.$(EXT)

blankresult blankresults:
	@-mkdir -p $(BIN)
	touch $(BIN)/results

eval-loc:
	@touch $(BIN)/results
	echo -n "Halide loc: " >> $(BIN)/results
	wc -l $(TESTNAME)_generator.cpp >> $(BIN)/results
	echo -n "Clockwork memory:  " >> $(BIN)/results
	wc -l $(BIN)/$(TESTNAME)_memory.cpp >> $(BIN)/results
	echo -n "Clockwork compute: " >> $(BIN)/results
	wc -l $(BIN)/$(TESTNAME)_compute.h >> $(BIN)/results
	echo -n "App mapped: " >> $(BIN)/results
	wc -l $(BIN)/design_top.json >> $(BIN)/results

eval-resources eval-rsrcs eval-area:
	python $(HWSUPPORT)/eval_design_top.py $(BIN)/design_top.json --resources | tee -a $(BIN)/results >&1

eval-latency:
	python $(HWSUPPORT)/eval_design_top.py $(BIN)/design_top.json --latency | tee -a $(BIN)/results >&1

eval-designtop:
	python $(HWSUPPORT)/eval_design_top.py $(BIN)/design_top.json --resources --latency | tee -a $(BIN)/results >&1

eval-complexity eval-memory:
	python $(HWSUPPORT)/eval_memory.py $(BIN)/$(TESTNAME)_memory.cpp | tee -a $(BIN)/results >&1

eval-ctime eval-compiler eval-compile_time:
	{ time $(MAKE) halide; } 2> >(tee -a $(BIN)/results >&2)
	@echo "^timing for 'make halide'" | tee -a $(BIN)/results >&1
	{ time $(MAKE) clockwork; } 2> >(tee -a $(BIN)/results >&2)
	@echo "^timing for 'make clockwork'" | tee -a $(BIN)/results >&1
	{ time $(MAKE) mem; } 2> >(tee -a $(BIN)/results >&2)
	@echo "^timing for 'make mem'" | tee -a $(BIN)/results >&1

eval-files:
	printf '\n### lines of code ###\n' >> $(BIN)/results
	$(MAKE) eval-loc
	printf '\n### resource usage ###\n' >> $(BIN)/results
	$(MAKE) eval-designtop
	printf '\n### complexity ###\n' >> $(BIN)/results
	$(MAKE) eval-memory

eval-all: $(BIN)/halide_gen_args
	@-mkdir -p $(BIN)
	@touch $(BIN)/results
	printf '\n\n########Starting new evaluation##########\n' >> $(BIN)/results
	date >> $(BIN)/results
	echo 'halide_gen_args:' >> $(BIN)/results
	cat $(BIN)/halide_gen_args >> $(BIN)/results
	printf '\n### compile time ###\n' >> $(BIN)/results
	$(MAKE) eval-ctime
	printf '\n### lines of code ###\n' >> $(BIN)/results
	$(MAKE) eval-loc
	printf '\n### resource usage ###\n' >> $(BIN)/results
	$(MAKE) eval-designtop
	printf '\n### complexity ###\n' >> $(BIN)/results
	$(MAKE) eval-memory

update_golden updategolden golden: $(BIN)/output_cpu.$(EXT) $(BIN)/$(TESTNAME)_memory.cpp
	@-mkdir -p $(GOLDEN)
	cp $(BIN)/output_cpu.$(EXT) $(GOLDEN)/golden_output.$(EXT)
	cp $(BIN)/$(TESTNAME)_memory.cpp $(GOLDEN)/$(TESTNAME)_memory.cpp
	cp $(BIN)/$(TESTNAME)_memory.h $(GOLDEN)/$(TESTNAME)_memory.h
	cp $(BIN)/$(TESTNAME)_compute.h $(GOLDEN)/$(TESTNAME)_compute.h

check:
	@printf "%-24s" $(TESTNAME);
	@if [ -f "$(BIN)/$(TESTNAME).generator" ]; then \
	  printf "  \033[0;32m%s\033[0m" " halide"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!halide"; \
	fi
#	@if [ -f "$(BIN)/extraction_debug.txt" ]; then \
	  printf "  \033[0;32m%s\033[0m" " extract"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!extract"; \
	fi
#	@if [ -f "$(BIN)/ubuffers.json" ]; then \
	  printf "  \033[0;32m%s\033[0m" " rewrite"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!rewrite"; \
	fi
#	@if [ -f "$(BIN)/design_prepass.json" ]; then \
	  printf "  \033[0;32m%s\033[0m" " coreir"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!coreir"; \
	fi
	@if [ -f "$(BIN)/$(TESTNAME)_memory.cpp" ]; then \
	  printf "  \033[0;32m%s\033[0m" " clkwork"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!clkwork"; \
	fi
	@if [ -f "$(BIN)/unoptimized_$(TESTNAME).cpp" ]; then \
	  printf "  \033[0;32m%s\033[0m" " unopt"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!unopt"; \
	fi
	@if [ -f "$(BIN)/output_cpu.$(EXT)" ]; then \
	  printf "  \033[0;32m%s\033[0m" " out_cpu"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!out_cpu"; \
	fi
	@if [ -f "$(BIN)/output_clockwork.$(EXT)" ]; then \
	  printf "  \033[0;32m%s\033[0m" " out_clk"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!out_clk"; \
	fi
	@if [ -f "$(BIN)/output.$(EXT)" ]; then \
	  printf "  \033[0;32m%s\033[0m" " out.png"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!out.png"; \
	fi
	@if [ -f "$(BIN)/map_result/$(TESTNAME)/$(TESTNAME).json" ]; then \
	  printf "  \033[0;32m%s\033[0m" " mapped"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!mapped"; \
	fi
	@if [ -f "$(BIN)/coreir_apps/single_port_buffer/$(TESTNAME)/$(TESTNAME).v" ]; then \
	  printf "  \033[0;32m%s\033[0m" " memtest"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!memtest"; \
	fi
#	@if [ -f "$(GOLDEN)/$(TESTNAME)_memory.cpp" ]; then \
	  printf "  \033[0;32m%s\033[0m" " golden"; \
	else \
	  printf "  \033[0;31m%s\033[0m" "!golden"; \
	fi
#	@@if [ -f "passed.md5" ]; then \
	  printf "  \033[0;32m%s\033[0m" "passed.md5"; \
	fi
#	@@if [ -f "failed.md5" ]; then \
	  printf "  \033[0;31m%s\033[0m" "failed.md5"; \
	fi
	@printf "\n"

list:
	@printf "%-24s\n" $(TESTNAME);

$(BIN)/graph.png: $(BIN)/design_top.txt
	dot -Tpng $(BIN)/design_top.txt > $(BIN)/graph.png
graph.png graph:
	$(MAKE) $(BIN)/graph.png
newgraph new_graph:
	cp $(BIN)/design_top.json $(BIN)/design_top_graph.json
	$(MAKE) graph

copy_design:
	docker cp setter-tender_lovelace:/aha/Halide-to-Hardware/apps/hardware_benchmarks/$(TESTORAPP)/$(TESTNAME)/bin/design.place bin/
	docker cp setter-tender_lovelace:/aha/Halide-to-Hardware/apps/hardware_benchmarks/$(TESTORAPP)/$(TESTNAME)/bin/design.route bin/
pnr_result: 
	python $(HWSUPPORT)/visualize_pnr.py bin 32 16

clean:
	rm -rf $(BIN) *_debug.csv test_results

clean-not-results:
	touch $(BIN)/results
	cp $(BIN)/results results && rm -rf $(BIN) *_debug.csv test_results && mkdir bin && cp results bin/results

test: run
