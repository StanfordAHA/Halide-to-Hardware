include ../../hw_support/Makefile.inc
ALIGN=./align
MOTION=./motion
BIN=./bin

all: test

$(BIN)/align.generator: $(ALIGN)/align.cpp $(GENERATOR_DEPS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)
	
$(BIN)/align.a: $(BIN)/align.generator
	@mkdir -p $(@D)
	$^ -g align -o $(BIN) -f align target=$(HL_TARGET)

$(BIN)/motion.generator: $(MOTION)/motion.cpp $(GENERATOR_DEPS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)
	
$(BIN)/motion.a: $(BIN)/motion.generator
	@mkdir -p $(@D)
	$^ -g motion -o $(BIN) -f motion_to_RGB -n motion target=$(HL_TARGET)

$(BIN)/run: run.cpp $(BIN)/align.a $(BIN)/motion.a
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(BIN) -Wall -O3 $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS)

test: $(BIN)/run
	@mkdir -p $(@D) output
	$(BIN)/run data 

clean:
	rm -rf $(BIN) output
