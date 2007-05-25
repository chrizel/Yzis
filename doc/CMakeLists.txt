
if (ENABLE_DOCUMENTATION)

    FIND_PACKAGE(Doxygen REQUIRED)
    message( "You can generate documentation with \"make docs\"" )    

	if(GENERATE_DOC)
        message( "Documentation is generated during build" )    
        set( ALWAYS_GENERATE ALL )
	endif(GENERATE_DOC)

    add_custom_target(
        docs ${ALWAYS_GENERATE}

        COMMAND ${DOXYGEN} devhandbook.doxy
        COMMAND ${DOXYGEN} libyzis.doxy
        COMMAND ${DOXYGEN} qyzis.doxy
        COMMAND ${DOXYGEN} nyzis.doxy
#        COMMAND ${DOXYGEN} kyzis.doxy

        # we need to add it twice to resolve all links
        COMMAND ${DOXYGEN} devhandbook.doxy
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
	  
endif (ENABLE_DOCUMENTATION)
