include ../../hw_support/Makefile.inc

TESTNAME = random_pipeline
USE_COREIR_VALID = 1
SEED ?= 1

include ../../hw_support/hardware_targets.mk

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

FLAGS_INT_BASIC_OPS=gen_div=true gen_mod=false gen_bool=true gen_cond=true
FLAGS_INT_POINTWISE=$(FLAGS_INT_BASIC_OPS) gen_conv1d=false gen_conv2d=false 
FLAGS_INT_CONV=$(FLAGS_INT_BASIC_OPS) gen_conv1d=true gen_conv2d=true
FLAGS_INT_UP=$(FLAGS_INT_CONV) gen_upsample=true
FLAGS_INT_DOWN=$(FLAGS_INT_UP) gen_downsample=true
FLAGS_INT_HIST=$(FLAGS_INT_DOWN) gen_histogram=true
FLAGS_INT_TOTAL=$(FLAGS_INT_HIST) gen_pool=true gen_activate=true gen_padding=true gen_all2all=true gen_scan=true

FLAGS_FLOAT_BASIC_OPS=$(FLAGS_INT_BASIC_OPS) gen_sin=true gen_tanh=false gen_exp=true gen_sqrt=true gen_log=true
FLAGS_FLOAT_POINTWISE=$(FLAGS_FLOAT_BASIC_OPS) gen_conv1d=false gen_conv2d=false
FLAGS_FLOAT_CONV=$(FLAGS_FLOAT_BASIC_OPS) gen_conv1d=true gen_conv2d=true
FLAGS_FLOAT_UP=$(FLAGS_FLOAT_CONV) gen_upsample=true
FLAGS_FLOAT_DOWN=$(FLAGS_FLOAT_UP) gen_downsample=true
FLAGS_FLOAT_HIST=$(FLAGS_FLOAT_DOWN) gen_histogram=true
FLAGS_FLOAT_TOTAL=$(FLAGS_FLOAT_HIST) gen_pool=true gen_activate=true gen_padding=true gen_all2all=true gen_scan=true

design-coreir-valid design-coreir_valid: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-coreir-coreir_valid -e coreir seed=$(SEED) \
	$(FLAGS_INT_POINTWISE)

design-coreir-int-pointwise: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-coreir-coreir_valid -e coreir seed=$(SEED) \
	$(FLAGS_INT_POINTWISE)

design-coreir-int-conv: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-coreir-coreir_valid -e coreir seed=$(SEED) \
	$(FLAGS_INT_CONV)

design-coreir-int-up: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-coreir-coreir_valid -e coreir seed=$(SEED) \
	$(FLAGS_INT_UP)

design-coreir-int-down: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-coreir-coreir_valid -e coreir seed=$(SEED) \
	$(FLAGS_INT_DOWN)

design-coreir-int-hist: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-coreir-coreir_valid -e coreir seed=$(SEED) \
	$(FLAGS_INT_HIST)

design-coreir-int-total: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)-coreir-coreir_valid -e coreir seed=$(SEED) \
	$(FLAGS_INT_TOTAL)

design-cpu-int-pointwise: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_INT_POINTWISE)

design-cpu-int-conv: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_INT_CONV)

design-cpu-int-up: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_INT_UP)

design-cpu-int-down: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_INT_DOWN)

design-cpu-int-hist: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_INT_HIST)

design-cpu-int-total: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_INT_TOTAL)

design-cpu-float-pointwise: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_FLOAT_POINTWISE)

design-cpu-float-conv: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_FLOAT_CONV)

design-cpu-float-up: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_FLOAT_UP)

design-cpu-float-down: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_FLOAT_DOWN)

design-cpu-float-hist: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_FLOAT_HIST)

design-cpu-float-total: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET) seed=$(SEED) \
	$(FLAGS_FLOAT_TOTAL)

