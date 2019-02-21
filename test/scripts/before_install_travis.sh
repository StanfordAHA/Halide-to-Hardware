#!/bin/bash -x

# Needed for new libstdc++ and gcc4.x
export CXX=g++
export CC=gcc
export LLVM_CONFIG=/usr/local/opt/llvm/lib/cmake/llvm/
export CLANG=/usr/local/opt/llvm/bin/
export COREIRCONFIG="g++"
export COREIR_DIR=/Users/dillon/CppWorkspace/coreir/
export OUTPUT_REDIRECTION=""
export HALIDE_SHARED_LIBARY="halide.a"
