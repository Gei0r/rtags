#!/bin/bash

VERSION="3.8.1"
LLVM_NAME=llvm-${VERSION}.src
LLVM_FILE=llvm-${VERSION}.src.tar.xz
CLANG_FILE=cfe-${VERSION}.src.tar.xz

STATUS=`curl -w "%{http_code}" -z ${LLVM_FILE} http://llvm.org/releases/${VERSION}/${LLVM_FILE} -o ${LLVM_FILE}`
if [ ${STATUS} == 200 ] || [ ! -d llvm-${VERSION}.src ]; then
    tar xf ${LLVM_FILE}
    curl -z ${CLANG_FILE} http://llvm.org/releases/${VERSION}/${CLANG_FILE} -o ${CLANG_FILE}
    tar xf ${CLANG_FILE}
    rm -f llvm-${VERSION}.src/tools/clang
    ln -s $PWD/cfe-${VERSION}.src llvm-${VERSION}.src/tools/clang
fi
