#!/bin/bash
#
# Author : Russ Barker
# Description : test IPC reader application
#

function pause()
{
    read -s -n 1 -p "Press any key to continue . . ."
    echo ""
}

clear

#curr_dir="$(dirname "$0")"
curr_dir=$(pwd)

echo "Exec root directory '${curr_dir}' "

cd ../TestData 

echo " "

../../build/ipcTest-reader --testOutputFile ./test-output.raw --logLevel 4 

pause
