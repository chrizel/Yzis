#!/bin/bash
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
# ./run_scripttests.sh -> execute all tests
# ./run_scripttests.sh test_movements.lua -> execute only "test_movements.lua"
# ./run_scripttests.sh qyzis --> execute all tests using the qyzis frontend
# ./run_scripttests.sh libyzisrunner --> execute all tests using libyzisrunner (No need for X-window)
# ./run_scripttests.sh gdb --> execute all tests from inside gdb with libyzisrunner
# ./run_scripttests.sh gdb test_movements.lua --> execute only "test_movements.lua" from inside gdb using libyzisrunner

# substituted by CMake during the build process

source ../env_test.sh 

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

yzis_arg1="-s"
yzis_arg2="$testname"

if [ "$YZIS" == "gdb" ];
then
	# execution in gdb
	LANG=C gdb $LIBYZISRUNNER_EXE -ex "set args $yzis_arg1 $yzis_arg2"
	exitcode=$?
else
	# normal execution
	echo LANG=C $YZIS $yzis_arg1 $yzis_arg2
	LANG=C $YZIS $yzis_arg1 "$yzis_arg2"
	exitcode=$?
	echo "$exitcode failed tests"
fi

source post_test.sh 

exit $exitcode

