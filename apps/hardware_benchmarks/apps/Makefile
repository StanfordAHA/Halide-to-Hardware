SHELL = /bin/bash
HALIDE_DEBUG_REDIRECT ?= &>/dev/null

default: all

# all directories (just text) excluding a particular list
ALL_APPS := $(subst /,, $(dir $(wildcard */.)))
ALL_APPS := $(filter-out hls_support tools, $(ALL_APPS))

### app suites include: ###
APP_SUITES := simple img dnn other

### simple applications ###
SIMPLE_APPS := pointwise gaussian cascade harris
simple := $(SIMPLE_APPS)

### imaging applications ###
# IMG_APPS := pointwise gaussian cascade harris \
#             example fast_corner unsharp \
#             demosaic demosaic_complex camera_pipeline \
#             exposure_fusion local_laplacian stereo hdr_plus
IMG_APPS := gaussian unsharp
img := $(IMG_APPS)

### DNN applications ###
# DNN_APPS := gemm unet \
#             conv_layer_2D conv_layer_3D conv_layer_mobile
# dnn := $(DNN_APPS)

### other applications ###
# OTHER_APPS := smith_waterman
# other := $(OTHER_APPS)

ALL_APPS_SORTED := $(IMG_APPS) $(DNN_APPS) $(OTHER_APPS)
allapps := $(ALL_APPS_SORTED)
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
    (exit $$EARLY_EXIT);  \
	fi
endef

# For each app suite, append the function to run
#  on each of the apps in that app suite.
#  For example, "make ops-coreir" creates the hardware for each op.
define SUITE_template =
$(1)-coreir:    $($(2):=-coreir)
$(1)-clockwork: $($(2):=-clockwork)
$(1)-run:       $($(2):=-run)
$(1)-compare:   $($(2):=-compare)
$(1)-compare-clockwork: $($(2):=-compare-clockwork)
$(1)-rewrite:   $($(2):=-rewrite)
$(1)-check:     $($(2):=-check)
$(1)-clean:     $($(2):=-clean)
endef

$(foreach suite,$(APP_SUITES),$(eval $(call SUITE_template,$(suite),$(suite))))
$(eval $(call SUITE_template,all,allapps))

%-coreir:
	@if [ -d $* ]; then \
			$(call run_app,$*,bin/design_top.json); \
	fi

%-clockwork:
	@if [ -d $* ]; then \
			$(call run_app,$*,compare-clockwork); \
	fi

%-run:
	@if [ -d $* ]; then \
			$(call run_app,$*,cpu); \
	fi

%-compare-clockwork:
	@if [ -d $* ]; then \
		$(call run_test,$*,compare-clockwork); \
	fi

%-compare:
	@if [ -d $* ]; then \
		$(call run_app,$*,compare); \
	fi

%-rewrite:
	@if [ -d $* ]; then \
		$(call run_test,$*,compare-rewrite); \
	fi

%-check:
	@if [ -d $* ]; then \
		$(MAKE) -sC $* check; \
	fi

%-clean:
	@if [ -d $* ]; then \
		$(call run_app,$*,clean); \
	fi

.PHONY: $(ALL_APPS)
$(ALL_APPS):
# 	@$(call run_app,$@,run)
# 	@if [ -f "$@/$(BIN)/output.png" ]; then \
# 		exit 1; \
# 	else \
# 		exit 0; \
# 	fi
# 	@$(call run_app,$@,compare-clockwork)
# 	@if [ -f "$@/$(BIN)/output_clockwork.png" ]; then \
# 		exit 1; \
# 	else \
# 		exit 0; \
# 	fi

all: testall

testall test_all: runall

testtravis test_travis: pointwise harris

list:
	@echo "APPS:"
	@echo $(ALL_APPS_SORTED)
	@echo "NON-CLASSIFIED APPS:"
	@echo $(NOT_CLASSIFIED_APPS)

runall run_all:$(ALL_APPS_SORTED)
$(ALL_APPS_SORTED):
# 	@$(call run_app,$@,run)
# 	@if [ -f "$@/$(BIN)/output.png" ]; then \
# 		exit 1; \
# 	else \
# 		exit 0; \
# 	fi
	@$(call run_app,$@,compare-clockwork)
	@if [ -f "$@/$(BIN)/output_clockwork.png" ]; then \
		exit 1; \
	else \
		exit 0; \
	fi

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

