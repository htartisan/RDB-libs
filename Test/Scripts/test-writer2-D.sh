#!/bin/bash
#
# Author : Russ Barker
# Description : test IPC writer2 application (debug build) 
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

../../build-dbg/ipcTest-writer2 --testInputFile ./sine440-1ch.raw --logLevel 5 

pause
