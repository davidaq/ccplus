#!/bin/bash
make testbuild
./build/test/out/Debug/test --gtest_filter="$1"
