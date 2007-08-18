
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/env_test.sh.in ${CMAKE_CURRENT_SOURCE_DIR}/env_test.sh
    @ONLY
)

if (WIN32) 
    # cygwin bash does not like dos line endings 
    EXECUTE_PROCESS( COMMAND conv --d2u ${CMAKE_CURRENT_SOURCE_DIR}/env_test.sh )
endif()

# add_subdirectory( libyzisrunner )
add_subdirectory( unittest )
add_subdirectory( scripts )

########### install files ###############
