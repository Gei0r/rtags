#!/bin/bash

echo $PWD > /tmp/fisk
VERSION="3.8.1"
LLVM=llvm-${VERSION}.src
LLVM_FILE=${LLVM}.tar.xz
CLANG=cfe-${VERSION}.src
CLANG_FILE=${CLANG}.tar.xz
LIBCXX=libcxx-${VERSION}.src
LIBCXX_FILE=${LIBCXX}.tar.xz

STATUS=`curl -w "%{http_code}" -z ${LLVM_FILE} http://llvm.org/releases/${VERSION}/${LLVM_FILE} -o ${LLVM_FILE}`
if [ ${STATUS} == 200 ] || [ ! -d ${LLVM} ]; then
    tar xf ${LLVM_FILE}
    curl -z ${CLANG_FILE} http://llvm.org/releases/${VERSION}/${CLANG_FILE} -o ${CLANG_FILE}
    tar xf ${CLANG_FILE}
    rm -f llvm-${VERSION}.src/tools/clang
    ln -s $PWD/${CLANG} llvm-${VERSION}.src/tools/clang

    if [ `uname -s` = Darwin ]; then
        curl -z ${LIBCXX_FILE} http://llvm.org/releases/${VERSION}/${LIBCXX_FILE} -o ${LIBCXX_FILE}
        tar xf ${LIBCXX_FILE}
        rm -f llvm-${VERSION}.src/projects/libcxx
        ln -s $PWD/${LIBCXX} llvm-${VERSION}.src/projects/libcxx
    fi
fi
