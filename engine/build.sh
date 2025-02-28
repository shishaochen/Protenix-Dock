#!/bin/bash
set -ex

# Declare global paths
dstdir=/opt/tiger/protenix_dock

# Build and test
if [ -f build/CMakeCache.txt ]; then
  cd build
  grep BDOCK CMakeCache.txt
else
  rm -rf build
  mkdir build
  cd build
  opts=-DBDOCK_AVX2=ON
  if [ "x${CUSTOM_BDOCK_DOUBLE}" != "x" ]; then
    opts="$opts -DBDOCK_DOUBLE=${CUSTOM_BDOCK_DOUBLE}"
  fi
  cmake .. $opts \
    -DBDOCK_UTESTS=${CUSTOM_BDOCK_UTESTS:-ON} \
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE:-Release} \
    -DCMAKE_INSTALL_PREFIX=$dstdir
  export VERBOSE=1
fi

# Collect binary & library
rm -rf $dstdir
make -j16 install
if [ -f bdopt_test ]; then
  ctest --output-on-failure
fi
if [ "x${BUILD_REPO_NAME}" != "x" ]; then
  cd ..
  rm -rf output
  mv $dstdir output
fi
