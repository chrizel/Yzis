
if (ENABLE_DOCUMENTATION)

    message( "Documentation generation is ON" )    
    FIND_PACKAGE(Doxygen REQUIRED)

	if(GENERATE_DOC)
        message( "Documentation generation is automatic" )    
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
