#!/bin/bash -x

# Download the right llvm release

LLVM_VERSION_NUM=`echo $LLVM_VERSION | cut -b 1`
if [[ ${LLVM_VERSION_NUM} -gt "5" ]]
then
    wget http://releases.llvm.org/${LLVM_VERSION}/clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-14.04.tar.xz
    tar xf clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-14.04.tar.xz
    mv clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-14.04 ${TRAVIS_BUILD_DIR}/llvm
else
    wget https://releases.llvm.org/${LLVM_VERSION}/clang+llvm-${LLVM_VERSION}-linux-x86_64-ubuntu14.04.tar.xz
    tar xf clang+llvm-${LLVM_VERSION}-linux-x86_64-ubuntu14.04.tar.xz
    mv clang+llvm-${LLVM_VERSION}-linux-x86_64-ubuntu14.04 ${TRAVIS_BUILD_DIR}/llvm
fi


