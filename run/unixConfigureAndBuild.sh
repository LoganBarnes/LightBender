#!/bin/bash

STRICT_FLAGS=OFF

function printUsage() {
  echo "Usage: ./unixConfigureAndBuild.sh <options>";
  echo "    options:";
  echo "        -sf or --strict-flags -> enable strict compile flags";
  echo "        -h  of --help         -> print this message";
}

# http://stackoverflow.com/questions/192249/how-do-i-parse-command-line-arguments-in-bash

# Use -gt 1 to consume two arguments per pass in the loop (e.g. each
# argument has a corresponding value to go with it).
# Use -gt 0 to consume one or more arguments per pass in the loop (e.g.
# some arguments don't have a corresponding value to go with it such
# as in the --default example).
# note: if this is set to -gt 0 the /etc/hosts part is not recognized ( may be a bug )
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -sf|--strict-flags)
    STRICT_FLAGS=ON
    ;;
    -h|--help)
    printUsage;
    exit;
    ;;
    *)
    echo "Unrecognized option: '$key'" # unknown option
    printUsage;
    exit;
    ;;
esac
shift # past argument or value
done

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
cmake -DCMAKE_BUILD_TYPE=$BUILD_MODE \
      -DBUILD_TESTS=$TESTING \
      -DSTRICT_FLAGS=$STRICT_FLAGS \
      -DCMAKE_INSTALL_PREFIX=$RUN_DIR ../..

cmake --build . --target install --config $BUILD_MODE -- -j12
# run the cmake build command to build the project with the native build system
cd ..
