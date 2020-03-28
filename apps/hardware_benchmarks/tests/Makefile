# all directories (just text) excluding a particular list
ALL_APPS := $(subst /,, $(dir $(wildcard */.)))
ALL_APPS := $(filter-out hls_support merged_unit_tests tools, $(ALL_APPS))

default: all

### test suites include: ###
# ops fpops convs fpconvs inouts mems
TEST_SUITES := ops fpops conv fpconv inout mem

### operator tests ###
OP_TESTS := absolute arith bitwise boolean_ops equal ternary \
            scomp sminmax sshift ucomp uminmax ushift \
            counter
ops = $(OP_TESTS)
op_tests op-tests ops: $(OP_TESTS)

### floating point operator tests ###
FP_OP_TESTS := fp_arith fp_comp fp_round fp_rectifier \
               fp_exp fp_invtrig fp_trig
fpops := $(FP_OP_TESTS)
fp_op_tests fp-op-tests fpops: $(FP_OP_TESTS)

### convolutions ###
CONV_TESTS := conv_1_1 conv_1_2 conv_2_1  conv_2_2 conv_3_1 \
              conv_3_3 conv_4_4 conv_5_5 conv_6_6 conv_7_7
conv := $(CONV_TESTS)
conv_tests conv-tests convs conv: $(CONV_TESTS)

### floating point convolutions ###
FP_CONV_TESTS := fp_pointwise fp_conv_1_1 fp_conv_2_2 fp_conv_3_3  \
                 fp_conv_4_4 fp_conv_5_5 fp_conv_6_6 fp_conv_7_7
fpconv := $(FP_CONV_TESTS)
fp_conv_tests fp-conv-tests fpconvs: $(FP_CONV_TESTS)

### accelerator interface tests ###
INOUT_TESTS := conv_2input inout_fp inout_onebit tap_param
inout := $(INOUT_TESTS)
inout_tests inout-tests inouts inout: $(INOUT_TESTS)

### memory tile tests ###
MEM_TESTS := rom fifo \
             conv_rolled conv_2x \
             down_sample up_sample strided_conv \
             conv_3_3 db_3_3 \
             accumulation histogram multi_channel_conv \
             two_level_memory three_level_memory pond
mem := $(MEM_TESTS)
mem_tests mem-tests mems mem: $(MEM_TESTS)

### generator tests ###
GEN_TESTS := conv_gen random_pipeline

HALIDE_DEBUG_REDIRECT ?= &>/dev/null
SHELL = /bin/bash

ALL_APPS_SORTED := $(OP_TESTS) $(FP_OP_TESTS) $(CONV_TESTS) \
                   $(FP_CONV_TESTS) $(INOUT_TESTS) $(MEM_TESTS)
NOT_CLASSIFIED_APPS := $(filter-out $(ALL_APPS_SORTED), $(ALL_APPS))

# Runs one of the functions on an app. Prints out nothing but pass/fail.
# Default exit 0 on failure so it doesn't stop a make script
EARLY_EXIT ?= 0
define run_app
	make -C $(1) $(2) $(HALIDE_DEBUG_REDIRECT); \
	EXIT_CODE=$$?; \
	if [[ $$EXIT_CODE = 0 ]]; then \
	  printf "%-25s \033[0;32m%s\033[0m\n" $(1) "PASSED"; \
    (exit $$EXIT_CODE); \
	else \
	  printf "%-25s \033[0;31m%s\033[0m\n" $(1) "FAILED"; \
    (exit $$EXIT_FAIL);  \
	fi
endef

.PHONY: $(ALL_APPS)
$(ALL_APPS):
	@$(call run_app,$@,bin/design_top.json)

# For each test suite, append the function to run
#  on each of the apps in that test suite.
#  For example, "make ops-coreir" creates the hardware for each op.
define SUITE_template =
$(1)-coreir:  $($(1):=-coreir)
$(1)-run:     $($(1):=-run)
$(1)-compare: $($(1):=-compare)
$(1)-check:   $($(1):=-check)
$(1)-clean:   $($(1):=-clean)
endef

$(foreach suite,$(TEST_SUITES),$(eval $(call SUITE_template,$(suite))))

%-coreir:
	@if [ -d $* ]; then \
			$(call run_app,$*,bin/design_top.json); \
	fi

%-run:
	@if [ -d $* ]; then \
			$(call run_app,$*,bin/output_coreir.png); \
	fi

%-compare:
	@if [ -d $* ]; then \
		$(call run_app,$*,compare); \
	fi

%-check:
	@if [ -d $* ]; then \
		$(MAKE) -sC $* check; \
	fi

%-clean:
	@if [ -d $* ]; then \
		$(call run_app,$*,clean); \
	fi

#@$(call run_app,$*,bin/design_top.json)
#	@if [ -d $* ]; then \
#		$(call run_app,$*,bin/design_top.json); \
#  else \
#		for app in $($*); do \
#			$(MAKE) -s $$app-coreir; \
#		done \
#	fi



#	@$(call run_app,$*,bin/design_top.json)

list:
	@echo "TESTS:"
	@echo $(ALL_APPS_SORTED)
	@echo "NON-CLASSIFIED APPS:"
	@echo $(NOT_CLASSIFIED_APPS)

all: $(ALL_APPS_SORTED)

testtravis test_travis: conv_3_3
testall test_all: runall
runall run_all: $(ALL_APPS)
compileall compile_all coreirall coreir_all: $(ALL_APPS:=-coreir)

evalall eval_all:
	@for app in $(ALL_APPS); do \
	  echo -n "$$app: "; \
	  $(MAKE) -sC $$app eval; \
	done

updategoldens update_goldens goldens:
	@echo ""
	@for app in $(ALL_APPS); do \
	  echo -n "$$app: "; \
	  $(MAKE) -sC $$app update_golden; \
	  echo "updated golden files for $$app"; \
	done

checkall check_all check:
	@for app in $(ALL_APPS_SORTED); do \
	  $(MAKE) -sC $$app check; \
	done

clean_all cleanall:
	for app in $(ALL_APPS); do \
	  $(MAKE) -C $$app clean > /dev/null; \
	done

