#!/bin/bash
mkdir build
cd build
cmake ..
make gravity_align_3d

./gravity_align_3d  ../3D/capsule.obj  ../3D/capsule.mtl ../3D/
exit 0

