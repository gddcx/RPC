#!/bin/bash
debug=false
while getopts "d" opt; do
  case $opt in
    d)
      debug=true
      ;;
    \?)
      echo "Usage: $0 [-d]"
      exit 1
      ;;
  esac
done

BUILD_DIR=`pwd`
ROOT_DIR=`dirname ${BUILD_DIR}`
TMP_DIR=${BUILD_DIR}/tmp

mkdir -p ${TMP_DIR}
cd ${TMP_DIR}


cmake ${BUILD_DIR} \
-G "Unix Makefiles" \
-DROOT_DIR=${ROOT_DIR}

if ${debug}; then
    echo "debug mode"
fi

make -j4;