SHELL = /bin/bash
TRAVIS_APPS := pond_accum resnet_pond resnet_lake pond_and_mem

define check_for_file	
  if [ ! -f $(1)/bin/$(2) ]; then \
      echo "$(1) has no file: $(2)"; \
			exit 1; \
  else \
      echo "$(1) has design_top.json"; \
  fi
endef

.PHONY: $(TRAVIS_APPS)
$(TRAVIS_APPS):
	@if [ -d $@ ]; then \
			$(call check_for_file,$@,design_top.json); \
	fi
	make -C $@ bin/input.raw $(HALIDE_DEBUG_REDIRECT)
	make -C $@ bin/output_cpu.raw $(HALIDE_DEBUG_REDIRECT)

travis testtravis test_travis: $(TRAVIS_APPS)
