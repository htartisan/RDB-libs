#!/bin/bash
#
# Author : Russ Barker
# Description : build ipc-test modules (for Linux x64)
#

clear

#curr_dir="$(dirname "$0")"
curr_dir=$(pwd)

echo "Build root directory '${curr_dir}' "

echo "Setting environment variables... " 

#"../Scripts/set-local-env.sh" 

BUILD=${curr_dir}/../../build-dbg

echo "Verifying target build ('${BUILD}') directory... " 

if [ -d "${BUILD}" ]  
then
    echo "Directory '${BUILD}' already exists " 
else
    echo "Creating directory '${BUILD}' "
	mkdir ${BUILD}
fi

# echo "Cleaning up (any) old build files (rm -rf *)... " 

( cd ${BUILD} && rm -rf * )

echo "Configuring 'build' of IPC "test-reader" and "test-writer" modules for Linux ... " 

# Linux Configuration 

SOURCE=${curr_dir}/../IpcTest

echo "Executing build ... " 

# Linux Build comommand

( cd ${BUILD} && cmake ${SOURCE} -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_TARGET_CPU="x86_64" -DCMAKE_BUILD_TYPE=Debug && cmake --build . --target all -- -j 8 $1 $2 $3 )



