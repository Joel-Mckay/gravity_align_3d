#!/bin/bash
mkdir build
cd build
cmake ..
make

./gravity_align_3d  ../3D/Rat.obj  ../3D/Rat.mtl ../3D/
exit 0

