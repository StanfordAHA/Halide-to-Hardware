#!/bin/bash -x

# Needed for new libstdc++ and gcc4.x
#export CXX=g++-4.9
#export CC=gcc-4.9
export CXX=g++-7
export CC=gcc-7
export LLVM_CONFIG=/usr/local/opt/llvm/lib/cmake/llvm/
export CLANG=/usr/local/opt/llvm/bin/
#export COREIRCONFIG="g++-4.9"
export COREIRCONFIG="g++-7"
export COREIR_DIR=/Users/dillon/CppWorkspace/coreir/
export OUTPUT_REDIRECTION=""
export HALIDE_SHARED_LIBARY="halide.a"
