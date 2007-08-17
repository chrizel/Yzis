#!/bin/sh

nb_failed=0

echo "Running unit tests"
(cd unittest && ./run_unittests.sh)
nb_unittest_failed=$?

echo "Running lua tests (libyzisrunner)"
(cd scripts && ./run_scripttests.sh)
nb_scripttest_libyr_failed=$?

echo "Running lua tests (qyzis)"
(cd scripts && ./run_scripttests.sh qyzis)
nb_scripttest_qyzis_failed=$?

echo
echo "======================[ Global Status ]================================"

nb_failed=$(( $nb_unittest_failed + $nb_scripttest_libyr_failed + $nb_scripttest_qyzis_failed ))

echo "               Unit tests: $nb_unittest_failed failed tests"
echo "Lua tests (libyzisrunner): $nb_scripttest_libyr_failed failed tests"
echo "        Lua tests (qyzis): $nb_scripttest_qyzis_failed failed tests"
echo
echo "Total: $nb_failed failed tests"

exit $nb_failed

