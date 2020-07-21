SHELL = /bin/bash
HALIDE_DEBUG_REDIRECT ?= &>/dev/null

default: all

# all directories (just text) excluding a particular list
ALL_TESTS := $(subst /,, $(dir $(wildcard */.)))
ALL_TESTS := $(filter-out hls_support merged_unit_tests tools, $(ALL_TESTS))

### test suites include: ###
# ops fpops convs fpconvs inouts mems
TEST_SUITES := ops fpops conv fpconv inout mem

### operator tests ###
# OP_TESTS := absolute arith bitwise boolean_ops equal ternary \
#             scomp sminmax sshift ucomp uminmax ushift \
#             counter
OP_TESTS := accumulation 
ops := $(OP_TESTS)
op_tests op-tests ops: $(OP_TESTS)

### floating point operator tests ###
# FP_OP_TESTS := fp_arith fp_comp fp_round fp_rectifier \
#                fp_exp fp_invtrig fp_trig
# fpops := $(FP_OP_TESTS)
# fp_op_tests fp-op-tests fpops: $(FP_OP_TESTS)

### convolutions ###
# CONV_TESTS := conv_1_1 conv_1_2 conv_2_1  conv_2_2 conv_3_1 \
#               conv_3_3 conv_4_4 conv_5_5 conv_6_6 conv_7_7
CONV_TESTS := conv_1_2 conv_2_1 conv_multi strided_conv
conv := $(CONV_TESTS)
conv_tests conv-tests convs conv: $(CONV_TESTS)

### floating point convolutions ###
# FP_CONV_TESTS := fp_pointwise fp_conv_1_1 fp_conv_2_2 fp_conv_3_3  \
#                  fp_conv_4_4 fp_conv_5_5 fp_conv_6_6 fp_conv_7_7
# fpconv := $(FP_CONV_TESTS)
# fp_conv_tests fp-conv-tests fpconvs: $(FP_CONV_TESTS)

### accelerator interface tests ###
# INOUT_TESTS := conv_2input inout_fp inout_onebit tap_param
# inout := $(INOUT_TESTS)
# inout_tests inout-tests inouts inout: $(INOUT_TESTS)

### memory tile tests ###
# MEM_TESTS := rom fifo \
#              conv_rolled conv_2x \
#              down_sample up_sample strided_conv \
#              conv_3_3 db_3_3 \
#              accumulation histogram multi_channel_conv \
#              two_level_memory three_level_memory pond
# mem := $(MEM_TESTS)
# mem_tests mem-tests mems mem: $(MEM_TESTS)

### generator tests ###
# GEN_TESTS := conv_gen random_pipeline


ALL_TESTS_SORTED := $(OP_TESTS) $(FP_OP_TESTS) $(CONV_TESTS) \
                   $(FP_CONV_TESTS) $(INOUT_TESTS) $(MEM_TESTS)
alltests := $(ALL_TESTS_SORTED)
NOT_CLASSIFIED_TESTS := $(filter-out $(ALL_TESTS_SORTED), $(ALL_TESTS))

# Runs one of the functions on an test. Prints out nothing but pass/fail.
# Default exit 0 on failure so it doesn't stop a make script
EARLY_EXIT ?= 0
define run_test
	make -C $(1) $(2) $(HALIDE_DEBUG_REDIRECT); \
	EXIT_CODE=$$?; \
	if [[ $$EXIT_CODE = 0 ]]; then \
	  printf "%-25s \033[0;32m%s\033[0m\n" $(1) "PASSED"; \
    (exit $$EXIT_CODE); \
	else \
	  printf "%-25s \033[0;31m%s\033[0m\n" $(1) "FAILED"; \
    (exit $$EARLY_EXIT);  \
	fi
endef

# For each test suite, testend the function to run
#  on each of the tests in that test suite.
#  For example, "make ops-clockwork" creates the hardware for each op.
define SUITE_template =
$(1)-clockwork:  $($(2):=-clockwork)
$(1)-run:     $($(2):=-run)
$(1)-compare-clockwork: $($(2):=-compare-clockwork)
$(1)-check:   $($(2):=-check)
$(1)-clean:   $($(2):=-clean)
endef

$(foreach suite,$(TEST_SUITES),$(eval $(call SUITE_template,$(suite),$(suite))))
$(eval $(call SUITE_template,all,alltests))

%-clockwork:
	@if [ -d $* ]; then \
			$(call run_test,$*,clockwork); \
	fi

%-run:
	@if [ -d $* ]; then \
			$(call run_test,$*,cpu); \
	fi

%-compare-clockwork:
	@if [ -d $* ]; then \
		$(call run_test,$*,compare-clockwork); \
	fi

%-clean:
	@if [ -d $* ]; then \
		$(call run_test,$*,clean); \
	fi

#@$(call run_test,$*,bin/design_top.json)
#	@if [ -d $* ]; then \
#		$(call run_test,$*,bin/design_top.json); \
#  else \
#		for test in $($*); do \
#			$(MAKE) -s $$test-coreir; \
#		done \
#	fi

all: $(ALL_TESTS_SORTED)

.PHONY: $(ALL_TESTS)
$(ALL_TESTS):
	@$(call run_test,$@,clockwork)
	@if [ -f "$@/$(BIN)/output_clockwork.png" ]; then \
		exit 1; \
	fi
	@$(call run_test,$@,compare-clockwork)
	@if [ -f "$@/$(BIN)/output_compare.png" ]; then \
		exit 1; \
	else \
		exit 0; \
	fi


testtravis test_travis: conv_3_3 fp_arith
testall test_all: runall
runall run_all: $(ALL_TESTS)
compileall compile_all coreirall coreir_all: $(ALL_TESTS:=-coreir)

list:
	@echo "TESTS:"
	@echo $(ALL_TESTS_SORTED)
	@echo "NON-CLASSIFIED TESTS:"
	@echo $(NOT_CLASSIFIED_TESTS)

evalall eval_all:
	@for test in $(ALL_TESTS); do \
	  echo -n "$$test: "; \
	  $(MAKE) -sC $$test eval; \
	done

updategoldens update_goldens goldens:
	@echo ""
	@for test in $(ALL_TESTS); do \
	  echo -n "$$test: "; \
	  $(MAKE) -sC $$test update_golden; \
	  echo "updated golden files for $$test"; \
	done

checkall check_all check:
	@for test in $(ALL_TESTS_SORTED); do \
	  $(MAKE) -sC $$test check; \
	done

clean_all cleanall:
	for test in $(ALL_TESTS); do \
	  $(MAKE) -C $$test clean > /dev/null; \
	done

