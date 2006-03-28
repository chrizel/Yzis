#!/bin/sh
# Usage:
# ./runtests.sh -> execute all tests
# ./runtests.sh test_movements.lua -> execute only test_movements.lua

testname='test_all.lua'
if [ -n "$1" ]; 
then
    testname=$1
fi

# save config file
if [ -f ~/.yzis/yzis.conf ]; then
	mv ~/.yzis/yzis.conf ~/.yzis/yzis.conf.old
fi
if [ -f ~/.yzis/init.lua ]; then
	mv -f ~/.yzis/init.lua ~/.yzis/init.lua.old
fi

YZIS=kyzis
YZIS=libyzisrunner

LANG=C $YZIS -c ":source $testname <ENTER><ESC>:qall!<ENTER>"

# restore config file
if [ -f ~/.yzis/yzis.conf.old ]; then
	mv ~/.yzis/yzis.conf.old ~/.yzis/yzis.conf
fi
if [ -f ~/.yzis/init.lua.old ]; then
	mv -f ~/.yzis/init.lua.old ~/.yzis/init.lua
fi
