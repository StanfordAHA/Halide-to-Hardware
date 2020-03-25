# all directories (just text) excluding a particular list
ALL_APPS := $(subst /,, $(dir $(wildcard */.)))
ALL_APPS := $(filter-out hls_support merged_unit_tests tools, $(ALL_APPS))
ALL_APPS_COREIR := $(patsubst %, %-coreir, $(ALL_APPS))

### test suites include:
# ops fpops convs fpconvs inouts mems

OP_TESTS := absolute arith bitwise boolean_ops equal ternary \
            scomp sminmax sshift ucomp uminmax ushift
op_tests op-tests ops: $(OP_TESTS)

FP_OP_TESTS := fp_arith fp_comp fp_round fp_rectifier \
               fp_exp fp_invtrig fp_trig
fp_op_tests fp-op-tests fpops: $(FP_OP_TESTS)

CONV_TESTS := conv_1_1 conv_1_2 conv_2_1  conv_2_2 conv_3_1 \
              conv_3_3 conv_4_4 conv_5_5 conv_6_6 conv_7_7
conv_tests conv-tests convs: $(CONV_TESTS)

FP_CONV_TESTS := fp_conv_1_1 fp_conv_2_2 fp_conv_3_3 fp_conv_4_4 \
                 fp_conv_5_5 fp_conv_6_6 fp_conv_7_7
fp_conv_tests fp-conv-tests fpconvs: $(FP_CONV_TESTS)

INOUT_TESTS := conv_2input inout_fp inout_onebit tap_param
inout_tests inout-tests inouts inout: $(INOUT_TESTS)

MEM_TESTS := rom fifo \
             conv_rolled conv_2x \
             down_sample up_sample strided_conv \
             accumulation multi_channel_conv \
             two_level_memory three_level_memory pond
mem_tests mem-tests mems mem: $(MEM_TESTS)


GEN_TESTS := conv_gen random_pipeline

HALIDE_DEBUG_REDIRECT ?= &>/dev/null
SHELL = /bin/bash

#all: testall
all: ops fpops convs fpconvs inouts mems

# Default exit 0 on failure so it doesn't stop a make script
EARLY_EXIT ?= 0
define run_app
	@make -C $(1) $(2) $(HALIDE_DEBUG_REDIRECT); \
	EXIT_CODE=$$?; \
	if [[ $$EXIT_CODE = 0 ]]; then \
	  printf "%-20s \033[0;32m%s\033[0m\n" $(1) "PASSED"; \
    (exit $$EXIT_CODE); \
	else \
	  printf "%-20s \033[0;31m%s\033[0m\n" $(1) "FAILED"; \
    (exit $$EXIT_FAIL);  \
	fi
endef

.PHONY: $(ALL_APPS)
$(ALL_APPS):
	$(call run_app,$@,bin/design_top.json)

%-coreir:
	$(call run_app,$*,bin/design_top.json)

list:
	@echo $(ALL_APPS)

testall test_all: runall

testtravis test_travis: conv_3_3

runall run_all: $(ALL_APPS)
compileall compile_all coreirall coreir_all: $(ALL_APPS_COREIR)

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

checkall check_all:
	@echo ""
	@for app in $(ALL_APPS); do \
	  $(MAKE) -sC $$app check; \
	done

clean_all cleanall:
	for app in $(ALL_APPS); do \
	  $(MAKE) -C $$app clean > /dev/null; \
	done

