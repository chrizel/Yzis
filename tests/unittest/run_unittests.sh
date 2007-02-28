#!/bin/sh 
# Usage:
# ./runtests.sh [testname]
#
# Copies from the build tree the latest versions of testyzis
# Then execute the test name provided on the command line.
#

export TMP="d:/tmp"
export TEMP="d:/tmp"

if [ -n "$CYGWIN" ]; 
then
    _exe=".exe"
else
    _exe=""
fi


echo "Copying yzis files"
cp -a ../../build-cmake/libyzis/libyzis* . 
cp -a ../../build-cmake/tests/unittest/yzistest$_exe .

YZIS=testyzis$_exe

case "x$1" in
    ("xgdb")
        shift
        YZIS="gdb"
        ;;
esac

if [ -n "$*" ]; 
then
    testname=$*
fi

echo "Testing with: $YZIS $testname"

if [ "$YZIS" == "gdb" ];
then
    # execution in gdb
    LANG=C gdb yzistest$_exe $testname
else
    # normal execution
    LANG=C ./yzistest$_exe $testname
fi


