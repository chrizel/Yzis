#!/bin/bash 
# Usage:
# ./runtests.sh [testname]
#
# shortcut to run the unittests from the source tree


source ../env_test.sh


case "x$1" in
    ("xgdb")
        shift
        GDB="gdb"
        ;;
esac

if [ -n "$*" ]; 
then
    testname=$*
fi

LANG=C $GDB $UNITTEST_EXE $testname

nb_failure=$?

echo "$nb_failure failed tests"

exit $nb_failure

