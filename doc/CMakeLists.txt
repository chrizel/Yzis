
if (ENABLE_DOCUMENTATION)

	FIND_PACKAGE(Doxygen REQUIRED)
	message( "You can generate documentation with \"make docs\"" )

	if(GENERATE_DOC)
		message( "Documentation is generated during build" )
		set( ALWAYS_GENERATE ALL )
	endif(GENERATE_DOC)

    FILE( MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../apidoc )

	add_custom_target(
		doc_libyzis
		COMMAND ${DOXYGEN} libyzis.doxy
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building libyzis api documentation"
	)

	add_custom_target(
		doc_qyzis
		COMMAND ${DOXYGEN} qyzis.doxy
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building qyzis api documentation"
	)
	add_custom_target(
		doc_nyzis
		COMMAND ${DOXYGEN} nyzis.doxy
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building nyzis api documentation"
	)
	add_custom_target(
		doc_devhandbook
		COMMAND ${DOXYGEN} devhandbook.doxy
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building dev handbook documentation"
	)

	add_custom_target(
		docs ${ALWAYS_GENERATE}
	)

	add_dependencies(
		docs doc_devhandbook doc_libyzis doc_qyzis doc_nyzis doc_devhandbook
	)


endif (ENABLE_DOCUMENTATION)
