#!/bin/sh
# Usage:
# ./runtests.sh -> execute all tests
# ./runtests.sh test_movements.lua -> execute only test_movements.lua

testname='test_all.lua'
if [ -n "$1" ]; 
then
    testname=$1
fi
LANG=C kyzis -c ":source $testname <ENTER><ESC>:qall!<ENTER>"
