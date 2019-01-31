#!/bin/bash -x

# Download the right llvm release
wget https://releases.llvm.org/${LLVM_VERSION}/clang+llvm-${LLVM_VERSION}-linux-x86_64-ubuntu14.04.tar.xz
tar xf clang+llvm-${LLVM_VERSION}-linux-x86_64-ubuntu14.04.tar.xz
mv clang+llvm-${LLVM_VERSION}-x86_64-linux-ubuntu-14.04 ${TRAVIS_BUILD_DIR}/llvm

