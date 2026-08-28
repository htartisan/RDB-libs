#!/bin/bash
#
# Author : Russ Barker
# Description : test writer application
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

../../build/ipcTest-writer2 --testInputFile ./67_wind_ensemble_mozart_mono.raw --logLevel 4 

pause
