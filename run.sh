#!/bin/bash

HERE=$(pwd)
# Make openssl
pushd openssl
./config --prefix=${HERE}/usr/local
make -sj4
make install_sw
popd

pushd libunwind
./autogen.sh
./configure --enable-debug #--disable-cxx-exceptions --enable-minidebuginfo=no
make -sj4
make install prefix=${HERE}/usr/local
popd

gcc -g -o test_bt test_bt.cpp -l:libcrypto.a -l:libunwind.a -lstdc++ -lrt -llzma -L ${HERE}/usr/local/lib -I ${HERE}/usr/local/include
