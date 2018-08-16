include ../../hw_support/Makefile.inc

BIN ?= bin

all: $(BIN)/process

$(BIN)/conv_3_1.generator: conv_3_1_generator.cpp $(GENERATOR_DEPS)
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

$(BIN)/conv_3_1.a: $(BIN)/conv_3_1.generator
	@-mkdir -p $(BIN)
	$^ -g conv_3_1 -o $(BIN) -f conv_3_1 target=$(HL_TARGET)

$(BIN)/process: process.cpp $(BIN)/conv_3_1.a 
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -I$(BIN) -Wall -O3 $^ -o $@ $(LDFLAGS)

run: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process

clean:
	rm -rf $(BIN)

test: run
