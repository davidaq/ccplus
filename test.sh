#!/bin/bash
make testbuild && ./build/out/Debug/test --gtest_filter="$1"
