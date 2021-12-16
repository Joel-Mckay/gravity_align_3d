#!/bin/bash
mkdir build
cd build
cmake ..
make

./gravity_align_3d  ../3D/cat.obj  ../3D/cat.obj.mtl ../3D/
exit 0

