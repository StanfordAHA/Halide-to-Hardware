# Usage:
# The following variables should be defined:
# $(APP_SUITES) := named groups of apps in the folder
# $(ALL_APPS_SORTED) := all apps sorted by suite that work
# $(ALL_APPS) := all apps, including those that don't work

allapps := $(ALL_APPS_SORTED)
all: $(ALL_APPS_SORTED)
testall test_all: runall
runall run_all: $(ALL_APPS_SORTED)
coreirall coreir_all: $(ALL_APPS:=-coreir)
compileall compile_all clockworkall clockwork_all clockwork-all: $(ALL_APPS_SORTED:=-clockwork)
compareall compare_all compare-all: $(ALL_APPS_SORTED:=-compare-clockwork)

testtravis:
	@echo TRAVIS TESTS: $(travis)
	$(MAKE) clearresults && $(MAKE) travis && $(MAKE) checkresults

suites suite:
	@-echo $(APP_SUITES)

halide compiler:
	$(MAKE) -C ../../.. quick_distrib

checkresults check_results:
	cat results
	if grep -q FAILED "results"; then \
		exit 1; \
	fi

clearresults clear_results:
	rm -f results

# Runs one of the functions on an app. Prints out nothing but pass/fail.
# Default exit 0 on failure so it doesn't stop a make script
EARLY_EXIT ?= 0
define run_app
	make -C $(1) $(2) $(HALIDE_DEBUG_REDIRECT); \
	EXIT_CODE=$$?; \
	if [[ $$EXIT_CODE = 0 ]]; then \
	  printf "%-25s \033[0;32m%s\033[0m\n" $(1) "PASSED"; \
	  printf "%-25s %s\n" $(1) "PASSED" | cat >> results; \
    (exit $$EXIT_CODE); \
	else \
	  printf "%-25s \033[0;31m%s\033[0m\n" $(1) "FAILED"; \
	  printf "%-25s %s\n" $(1) "FAILED" | cat >> results; \
    (exit $$EARLY_EXIT);  \
	fi
endef

# For each app suite, append the function to run
#  on each of the apps in that app suite.
#  For example, "make ops-clockwork" creates the hardware for each op.
define SUITE_template =
$(1)-cpu:               $($(2):=-cpu)
$(1)-coreir:            $($(2):=-coreir)
$(1)-clockwork:         $($(2):=-clockwork)
$(1)-run-cpu:           $($(2):=-run-cpu)
$(1)-compare:           $($(2):=-compare)
$(1)-compare-clockwork: $($(2):=-compare-clockwork)
$(1)-compare-coreir:    $($(2):=-compare-coreir)
$(1)-mem:               $($(2):=-mem)
$(1)-memtest:           $($(2):=-memtest)
$(1)-rewrite:           $($(2):=-rewrite)
$(1)-check:             $($(2):=-check)
$(1)-list:              $($(2):=-list)
$(1)-golden:            $($(2):=-golden)
$(1)-clean:             $($(2):=-clean)
endef

$(foreach suite,$(APP_SUITES),$(eval $(call SUITE_template,$(suite),$(suite))))
$(eval $(call SUITE_template,all,allapps))

%-cpu:
	@if [ -d $* ]; then \
			$(call run_app,$*,cpu); \
	fi

%-coreir:
	@if [ -d $* ]; then \
			$(call run_app,$*,bin/design_top.json); \
	fi

%-clockwork:
	@if [ -d $* ]; then \
			$(call run_app,$*,clockwork); \
	fi

%-run-cpu:
	@if [ -d $* ]; then \
			$(call run_app,$*,run-cpu); \
	fi

%-compare-clockwork %-cclockwork:
	@if [ -d $* ]; then \
		$(call run_app,$*,compare-clockwork); \
	fi

%-compare:
	@if [ -d $* ]; then \
		$(call run_app,$*,compare); \
	fi

%-mem:
	@if [ -d $* ]; then \
		$(call run_app,$*,mem); \
  fi

%-memtest:
	@if [ -d $* ]; then \
		$(call run_app,$*,memtest); \
  fi

%-rewrite:
	@if [ -d $* ]; then \
		$(call run_app,$*,compare-rewrite); \
	fi

%-golden:
	@if [ -d $* ]; then \
		$(call run_app,$*,golden); \
	fi

%-check:
	@if [ -d $* ]; then \
		$(MAKE) -sC $* check; \
	fi

%-list:
	@if [ -d $* ]; then \
		$(MAKE) -sC $* list; \
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

.PHONY: $(ALL_APPS)
$(ALL_APPS):
# 	@$(call run_app,$@,clockwork)
# 	@if [ -f "$@/$(BIN)/output_clockwork.png" ]; then \
# 		exit 1; \
# 	fi
	@$(call run_app,$@,compare-clockwork)
	@if [ -f "$@/$(BIN)/output.png" ]; then \
		exit 1; \
	else \
		exit 0; \
	fi

UNCLASSIFIED_APPS := $(filter-out $(ALL_APPS_SORTED), $(ALL_APPS))
list:
	@echo "APPS:"
	@echo $(ALL_APPS_SORTED)
	@echo "UNCLASSIFIED APPS:"
	@echo $(UNCLASSIFIED_APPS)

evalall eval_all:
	@for app in $(ALL_APPS_SORTED); do \
	  echo -n "$$app: "; \
	  $(MAKE) -sC $$app eval; \
	done

updategoldens update_goldens goldens:
	@echo ""
	@for app in $(ALL_APPS_SORTED); do \
	  echo -n "$$app: "; \
	  $(MAKE) -sC $$app update_golden; \
	  echo "updated golden files for $$app"; \
	done

checkall check_all check:
	@for app in $(ALL_APPS_SORTED); do \
	  $(MAKE) -sC $$app check; \
	done

MISSING_APPS := $(filter-out $(ALL_APPS_SORTED), $(ALL_APPS))

clean_all cleanall:
	for app in $(ALL_APPS_SORTED); do \
	  $(MAKE) -C $$app clean > /dev/null; \
	done

