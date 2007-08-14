#
# This bash script is run before running a test, to save the user yzis
# directory. This important to have a clean environment, and not to mess the
# user personal stuff.
# 
# post_test.sh must be run after the test.
#
# To use it, simply source it:
# source pre_test.sh
#
# protect user config file

source env_test.sh

if [ -n "$CYGWIN" ]; then
    user_home=`cygpath "$USERPROFILE"`;
else
    user_home="~";
	fi

if [ -e "$user_home/.yzis" ]; then
    echo "Saving $user_home/.yzis to .yzis_saved_for_tests"
    rm -rf "$user_home/.yzis_saved_for_tests"
    mv "$user_home/.yzis" "$user_home/.yzis_saved_for_tests"
    if [ "$?" != "0" ]; then
        echo "Could not save $user_home/.yzis directory, aborting"; 
        exit -1;
	fi
fi

# add current directory to LD_LIBRARY_PATH
old_ld_lib_path=$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$BUILD_DIR/libyzis:$LD_LIBRARY_PATH

