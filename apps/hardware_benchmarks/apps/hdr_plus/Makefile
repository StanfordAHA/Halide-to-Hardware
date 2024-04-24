include ../../hw_support/Makefile.inc

TESTNAME = hdr_plus
USE_COREIR_VALID = 1
HL_TARGET = host-x86-64
RDAI_PLATFORM_RUNTIME = clockwork_sim

include ../../hw_support/hardware_targets.mk

ALIGN=./align
MOTION=./motion
MERGE=./merge
FINISH=./finish
BIN=./bin

all: test

$(BIN)/align.generator: $(ALIGN)/align.cpp $(GENERATOR_DEPS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

$(BIN)/align.a: $(BIN)/align.generator
	@mkdir -p $(@D)
	$^ -g align -o $(BIN) -f align target=$(HL_TARGET)

$(BIN)/merge.generator: $(MERGE)/merge.cpp $(GENERATOR_DEPS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

$(BIN)/merge.a: $(BIN)/merge.generator
	@mkdir -p $(@D)
	$^ -g merge -o $(BIN) -f merge target=$(HL_TARGET)

$(BIN)/finish.generator: $(FINISH)/finish.cpp $(GENERATOR_DEPS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

$(BIN)/finish.a: $(BIN)/finish.generator
	@mkdir -p $(@D)
	$^ -g finish -o $(BIN) -f finish target=$(HL_TARGET)

$(BIN)/motion.generator: $(MOTION)/motion.cpp $(GENERATOR_DEPS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

$(BIN)/motion.a: $(BIN)/motion.generator
	@mkdir -p $(@D)
	$^ -g motion -o $(BIN) -f motion_to_RGB -n motion target=$(HL_TARGET)

#$(BIN)/process: process.cpp $(BIN)/align.a $(BIN)/motion.a $(BIN)/merge.a $(BIN)/finish.a
#	@mkdir -p $(@D)
#	$(CXX) $(CXXFLAGS) -I$(BIN) -Wall -O3 $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS)

test: $(BIN)/process
	@mkdir -p $(@D) output
	$(BIN)/process data 


raw: unprocessed_raw.cpp unprocessed_raw

unprocessed_raw: unprocessed_raw.o
	$(CXX) $(CXXFLAGS) -o $@ -I /usr/include/ unprocessed_raw.o -L /usr/lib/ -lraw

unprocessed_raw.cpp.o: 
	$(CXX) $(CXXFLAGS) $< -o $@


align_test:
	g++ -g -o align align/align.cpp

