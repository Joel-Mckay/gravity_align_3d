#!/bin/bash
mkdir build
cd build
cmake ..
make gridcells

./gridcells
exit 0

