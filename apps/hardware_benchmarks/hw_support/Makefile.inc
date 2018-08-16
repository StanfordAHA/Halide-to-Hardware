HALIDE_BIN_PATH ?= ../../../..
HALIDE_SRC_PATH ?= ../../../..
LDFLAGS ?=
BIN ?= ./bin
IMAGES ?= ../../../images

include ../../../support/Makefile.inc

LDFLAGS += -lcurses