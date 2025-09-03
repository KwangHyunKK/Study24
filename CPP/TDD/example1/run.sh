#!/bin/bash

CURRNT_PATH=$(pwd)
BUILD_PATH="$CURRNT_PATH/build"

cd $BUILD_PATH

cmake -S .. -B $BUILD_PATH -DBUILD_TESTING=ON -DENABLE_ASAN=ON
cmake --build $BUILD_PATH -j

# Run tests (ctest picks up gtest tests)
ctest --test-dir $BUILD_PATH --output-on failure