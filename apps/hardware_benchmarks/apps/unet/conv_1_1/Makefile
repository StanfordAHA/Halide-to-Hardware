HALIDE_BIN_PATH ?= $(abspath ../../../../..)
HALIDE_SRC_PATH ?= $(abspath ../../../../..)
HWSUPPORT ?= $(abspath ../../../hw_support)

# Usage:
#  make all:                    compiles all code without running
#       generator:              create Halide generator
#       design:                 create cpu design
#       design-clockwork:       create clockwork design
#       image:                  create an image with random data
#       run:                    run cpu design with image
#       run-clockwork:          run clockwork design with image
#       compare:                compare two output images
#       test:                   run and compare to cpu output
#       eval:                   evaluate runtime
#       clean:                  remove bin directory

###############################################################################
# MODIFICATION: NONE
#------------------------------------------------------------------------------
# Makefile parameter/variable declarations
#
###############################################################################

include $(HWSUPPORT)/Makefile.inc



###############################################################################
# MODIFICATION: OPTIONAL
#------------------------------------------------------------------------------
# App-specific info
#
# TESTNAME                      : name of the app
# USE_CORE_IR_VALID             : whether to generate valid signal for coreir codegen
# HL_TARGET                     : Halide target
# RDAI_PLATFORM_RUNTIME         : RDAI platform runtime to use 
###############################################################################

TESTNAME                        = conv_1_1
USE_COREIR_VALID                = 1
HL_TARGET                       = host-x86-64
RDAI_PLATFORM_RUNTIME           = clockwork_sim



###############################################################################
# MODIFICATION : NONE
#------------------------------------------------------------------------------
# Include hardwrae build targets
#
###############################################################################

include $(HWSUPPORT)/hardware_targets.mk

test-mem mem bin/design_top.json $(BIN)/map_result/$(TESTNAME)/$(TESTNAME).json:
	@echo "Doesn't map through clockwork. Takes too long."
	exit 1
