#!/bin/bash
mkdir build
cd build
cmake ..
make

./gravity_align_3d  ../3D/tiger.obj  ../3D/tiger.obj.mtl ../3D/
exit 0

