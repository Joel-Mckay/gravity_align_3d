#!/bin/bash
mkdir build
cd build
cmake ..
make gravity_align_3d

./gravity_align_3d  ../3D/Nefertiti.obj  ../3D/Nefertiti.obj.mtl ../3D/
exit 0

