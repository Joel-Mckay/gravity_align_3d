#!/bin/bash
mkdir build
cd build
cmake ..
make gravity_align_3d

./gravity_align_3d  ../3D/keymarcocat-150k.obj  ../3D/keymarcocat-150k.mtl ../3D/
exit 0

