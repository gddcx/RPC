#!/bin/bash

cur_dir=`pwd`
projectdir=`dirname ${cur_dir}`

cd ${projectdir}/build
bash build.sh
cd -

mkdir -p tmp
cd tmp

cmake ${cur_dir} \
-G "Unix Makefiles" \
-DROOT_DIR=${projectdir} \
-DLIB_NET=${projectdir}/build/tmp/libnetwork.a

make -j4

./network_ut
