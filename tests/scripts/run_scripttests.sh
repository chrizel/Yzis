#!/bin/sh 
# Usage:
# ./runtests.sh ["nocopy"] [Yzis frontend] [test name]
# Copies from the build tree the latest versions of nyzis, qyzis, libyzis and
# libyzisrunner. Then execute the test name provided on the command line with
# the frontend specified.
#
# If "nocopy" is specified, do not copy the compiled version from the cmake
# build tree priori to running the tests.
# If no frontend is given, execute the tests with libyzisrunner
# If no test name is specified, run all tests
#
# ./runtests.sh -> execute all tests
# ./runtests.sh test_movements.lua -> execute only test_movements.lua
# ./runtests.sh qyzis --> execute tests with qyzis frontend
# ./runtests.sh libyzisrunner --> execute tests with no gui yzis frontend
# ./runtests.sh gdb --> execute tests with in gdb with libyzisrunner
# ./runtests.sh gdb test_movements.lua --> execute tests with in gdb with libyzisrunner with test_movements.lua as argument

if [ -n "$CYGWIN" ]; then
    export TMP="d:/tmp"
    export TEMP="d:/tmp"
fi

if [ "X$1" != "Xnocopy" ];
then
    echo "Copying yzis files"
    if [ -n "$CYGWIN" ]; 
    then
        cp -a ../../build-cmake/libyzis/libyzis* . 
        cp -a ../../build-cmake/qyzis/qyzis.exe . 
        cp -a ../../build-cmake/libyzisrunner/libyzisrunner.exe .
    else
        cp -a ../../build-cmake/libyzis/libyzis* . 
        cp -a ../../build-cmake/qyzis/qyzis . 
        cp -a ../../build-cmake/nyzis/nyzis . 
        cp -a ../../build-cmake/libyzisrunner/libyzisrunner .
    fi
else
    echo "Not copying yzis files"
    shift
fi

YZIS="libyzisrunner"

case "x$1" in
    ("xqyzis")
        shift
        YZIS="qyzis"
        ;;

    ("xlibyzisrunner")
        shift
        YZIS="libyzisrunner"
        ;;

    ("xgdb")
        shift
        YZIS="gdb"
        ;;
esac

testname='test_all.lua'
if [ -n "$*" ]; 
then
    testname=$*
fi

# save config file on linux
if [ -z "$CYGWIN" ]; then
    if [ -f ~/.yzis/yzis.conf ]; then
        mv ~/.yzis/yzis.conf ~/.yzis/yzis.conf.old
    fi
    if [ -f ~/.yzis/init.lua ]; then
        mv -f ~/.yzis/init.lua ~/.yzis/init.lua.old
    fi
fi

echo "Testing with: $YZIS $testname"

yzis_arg1="-c"
yzis_arg2="\":source $testname <ENTER><ESC>:qall!<ENTER>\""

if [ "$YZIS" == "gdb" ];
then
    # execution in gdb
    LANG=C gdb libyzisrunner.exe -w -ex "set args $yzis_arg1 $yzis_arg2"
else
    # normal execution
    echo LANG=C ./$YZIS $yzis_arg1 $yzis_arg2
    LANG=C ./$YZIS $yzis_arg1 "$yzis_arg2"
fi

# restore config file on linux
if [ -z "$CYGWIN" ]; then
    if [ -f ~/.yzis/yzis.conf.old ]; then
        mv ~/.yzis/yzis.conf.old ~/.yzis/yzis.conf
    fi
    if [ -f ~/.yzis/init.lua.old ]; then
        mv -f ~/.yzis/init.lua.old ~/.yzis/init.lua
    fi
fi


