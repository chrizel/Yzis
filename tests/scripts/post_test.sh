#
# This bash script is run after a test has been run, to restore the user yzis
# directory. The script assumes that pre_test.sh has been run before.
#
# To use it, simply source it:
# source post_test.sh
#


# restore config file on linux
if [ -e "$user_home/.yzis_saved_for_tests" ]; then
    echo "Restoring .yzis_saved_for_tests to $user_home/.yzis"
    rm -rf "$user_home/.yzis"
    mv "$user_home/.yzis_saved_for_tests" "$user_home/.yzis"
fi

# restore original LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$old_ld_lib_path

