
if (ENABLE_DOCUMENTATION)

	FIND_PACKAGE(Doxygen REQUIRED)
	message( "You can generate documentation with \"make docs\"" )

	if(GENERATE_DOC)
		message( "Documentation is generated during build" )
		set( ALWAYS_GENERATE ALL )
	endif(GENERATE_DOC)

	add_custom_target(
		doc_libyzis
		COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/../apidoc
		COMMAND ${DOXYGEN} libyzis.doxy
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
	add_custom_target(
		doc_qyzis
		COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/../apidoc
		COMMAND ${DOXYGEN} qyzis.doxy
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
	add_custom_target(
		doc_nyzis
		COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/../apidoc
		COMMAND ${DOXYGEN} nyzis.doxy
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
	add_custom_target(
		doc_devhandbook
		COMMAND ${DOXYGEN} devhandbook.doxy
		# second time to get all the links right
		COMMAND ${DOXYGEN} devhandbook.doxy
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	add_custom_target(
		docs ${ALWAYS_GENERATE}
	)
	add_dependencies(
		docs doc_libyzis doc_qyzis doc_nyzis doc_devhandbook
	)


endif (ENABLE_DOCUMENTATION)
