#!/bin/sh
testname='test_all.lua'
if [ -n "$1" ]; 
then
    testname=$1
fi
kyzis -c ":source $testname <ENTER><ESC>:qall!<ENTER>"
