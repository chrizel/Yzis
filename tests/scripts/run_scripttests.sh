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

# substituted by CMake during the build process

source env_test.sh 

if [ "X$1" != "Xnocopy" ];
then
	echo "Copying yzis files"
	if [ -n "$CYGWIN" ];
	then
		cp -u $BUILD_DIR/libyzis/libyzis.dll .
	fi
else
	echo "Not copying yzis files"
	shift
fi

YZIS=$LIBYZISRUNNER_EXE

case "x$1" in
	("xqyzis")
	shift
	YZIS=$QYZIS_EXE
	;;

	("xlibyzisrunner")
	shift
	YZIS=$LIBYZISRUNNER_EXE
	;;

	("xgdb")
	shift
	YZIS="gdb"
	;;

	("xnyzis")
	shift
	YZIS=$NYZIS_EXE
	;;
esac

testname='test_all.lua'
if [ -n "$*" ];
then
	testname=$*
fi

source pre_test.sh 

echo "Testing with: $YZIS $testname"

yzis_arg1="-c"
yzis_arg2="\":source $testname <ENTER><ESC>:qall!<ENTER>\""

if [ "$YZIS" == "gdb" ];
then
	# execution in gdb
	LANG=C gdb $LIBYZISRUNNER_EXE -ex "set args $yzis_arg1 $yzis_arg2"
else
	# normal execution
	echo LANG=C $YZIS $yzis_arg1 $yzis_arg2
	LANG=C $YZIS $yzis_arg1 "$yzis_arg2"
fi

source post_test.sh 
