#!/bin/bash
make testbuild && \
    lldb -f ./build/out/Debug/test -o 'run --gtest_filter="'$1'"'
