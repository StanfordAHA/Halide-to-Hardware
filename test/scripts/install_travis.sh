#!/bin/bash -x

wget -q http://releases.llvm.org/${LLVM_VERSION}/clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-14.04.tar.xz
tar xf clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-14.04.tar.xz
mv clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-14.04 ${TRAVIS_BUILD_DIR}/llvm

# compile coreir
git clone -b ubuffer https://github.com/rdaly525/coreir.git
cd coreir/build && cmake ..
make -j2 all && cd ../../
git clone https://github.com/dillonhuff/BufferMapping.git
cd BufferMapping/cfunc/
make lib && cd ../../

