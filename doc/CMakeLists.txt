
if (ENABLE_DOCUMENTATION)

	FIND_PACKAGE(Doxygen REQUIRED)
	message( "You can generate documentation with \"make docs\"" )

	if(GENERATE_DOC)
		message( "Documentation is generated during build" )
		set( ALWAYS_GENERATE ALL )
	endif(GENERATE_DOC)

	# create the meta target docs which will later depend on all other
	# doc targets
	add_custom_target(
		docs ${ALWAYS_GENERATE}
		)

	SET( apidir ${CMAKE_BINARY_DIR}/apidoc )

	FILE( MAKE_DIRECTORY ${apidir} )

	MACRO(add_doc doc_part discription)
		# this is just that it can be accessed in the doxy.in files
		SET(documentation_part ${doc_part})

		# if the *.doxy.in file is not in the right location there is
		# an error
		if( NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${doc_part}.doxy.in" )
			message(SEND_ERROR "doxy file of ${doc_part} not found in doc" )
		endif()

		# this parses the doxy.in files and replaces @variable@ with
		# the value of the variable
		configure_file( ${doc_part}.doxy.in
			${CMAKE_CURRENT_BINARY_DIR}/${doc_part}.doxy )

		# this is the part that actually generates the doc targets
		add_custom_target(
			doc_${doc_part}
			COMMAND ${DOXYGEN}
			${CMAKE_CURRENT_BINARY_DIR}/${doc_part}.doxy
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			COMMENT discription
			)

		# here we just make sure this doc is generated then we run
		# "make docs"
		add_dependencies(
			docs doc_${doc_part}
			)
	ENDMACRO()


	add_doc(nyzis "Build nyzis api documentation")
	add_doc(qyzis "Build qyzis api documentation")
	add_doc(libyzis "Build qyzis api documentation")
	add_doc(devhandbook "Build dev Handbook documentation")

endif (ENABLE_DOCUMENTATION)
