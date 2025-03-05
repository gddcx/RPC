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
-DLIB_NET=${projectdir}/build/tmp/libnetwork.a \
-DLIB_RPC=${projectdir}/build/tmp/librpc.a \
-DLIB_PROTOCOL=${projectdir}/build/tmp/libprotocol.a \
-DLIB_KEEPER=${projectdir}/build/tmp/libkeeper.a \
-DLIB_MONITOR=${projectdir}/build/tmp/libmonitor.a \
-DLIB_UTILS=${projectdir}/build/tmp/libutils.a

make -j4
