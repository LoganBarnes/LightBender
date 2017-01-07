#!/bin/bash

# current run directory
RUN_DIR=$(pwd)

# toggleable variables
TESTING=OFF
BUILD_MODE=Release

# create the _build directory if it doesn't exist
if [[ ! -d _build ]];
  then
  mkdir _build;
fi;

# run cmake from the build directory to configure the project
cd _build
cmake -DCMAKE_BUILD_TYPE=$BUILD_MODE -DBUILD_TESTS=$TESTING -DCMAKE_INSTALL_PREFIX=$RUN_DIR ../..

cmake --build . --target install --config $BUILD_MODE -- -j12
# run the cmake build command to build the project with the native build system
cd ..
