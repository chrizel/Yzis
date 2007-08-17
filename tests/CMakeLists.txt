
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/env_test.sh.in ${CMAKE_CURRENT_SOURCE_DIR}/env_test.sh
    @ONLY
)

# add_subdirectory( libyzisrunner )
add_subdirectory( unittest )
add_subdirectory( scripts )

########### install files ###############
