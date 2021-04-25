#!/bin/bash -x

# Needed for new libstdc++ and gcc4.x
export CXX=${CXX_}
export CC=${CC_}
export LLVM_CONFIG=${TRAVIS_BUILD_DIR}/llvm/bin/llvm-config
export CLANG=${TRAVIS_BUILD_DIR}/llvm/bin/clang
#export COREIRCONFIG="g++-4.9"
#export COREIRCONFIG="g++-5"
export COREIRCONFIG=${CXX_}
export COREIR_DIR=${TRAVIS_BUILD_DIR}/coreir
export COREIR_PATH=${TRAVIS_BUILD_DIR}/coreir
export FUNCBUF_DIR=${TRAVIS_BUILD_DIR}/BufferMapping/cfunc
export RDAI_DIR=${TRAVIS_BUILD_DIR}/rdai
export OUTPUT_REDIRECTION=""

# Needed for clockwork
export BARVINOK_PATH=${TRAVIS_BUILD_DIR}/clockwork/barvinok-0.41/isl
export ISL_PATH=${TRAVIS_BUILD_DIR}/clockwork/barvinok-0.41/isl
export OPT_PATH=${TRAVIS_BUILD_DIR}/clockwork/include
export OPT_LIB_PATH=${TRAVIS_BUILD_DIR}/clockwork/lib
export CLKWRK_PATH=${TRAVIS_BUILD_DIR}/clockwork
export CLOCKWORK_DIR=${TRAVIS_BUILD_DIR}/clockwork
export LAKE_PATH=${TRAVIS_BUILD_DIR}/lake

export COREIR=1
export CGRAFLOW=1

