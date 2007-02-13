FIND_PACKAGE(Doxygen REQUIRED)

if (ENABLE_DOCUMENTATION)
	if(DOXYGEN_EXECUTABLE AND UNIX)
	  
		add_custom_command(
			OUTPUT ${CMAKE_SOURCE_DIR}/apidoc/index.html
			COMMAND sh build_doxygen_documentation.sh
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)

		add_custom_target(
			documentation ALL
			DEPENDS
			${CMAKE_SOURCE_DIR}/apidoc/index.html
		)
	  
	endif(DOXYGEN_EXECUTABLE AND UNIX)
endif (ENABLE_DOCUMENTATION)
