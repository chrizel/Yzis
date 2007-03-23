
if (ENABLE_DOCUMENTATION)
    FIND_PACKAGE(Doxygen REQUIRED)
	  
	if(GENERATE_DOC)
        set( ALWAYS_GENERATE ALL )
	endif(GENERATE_DOC)

    add_custom_target(
        docs ${ALWAYS_GENERATE}

        COMMAND ${DOXYGEN_EXECUTABLE} devhandbook.doxy
        COMMAND ${DOXYGEN_EXECUTABLE} libyzis.doxy
        COMMAND ${DOXYGEN_EXECUTABLE} qyzis.doxy
        COMMAND ${DOXYGEN_EXECUTABLE} nyzis.doxy
#        COMMAND ${DOXYGEN_EXECUTABLE} kyzis.doxy

        # we need to add it twice to resolve all links
        COMMAND ${DOXYGEN_EXECUTABLE} devhandbook.doxy
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
	  
endif (ENABLE_DOCUMENTATION)
