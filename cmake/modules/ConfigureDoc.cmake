# - Configure Documentation
# This module can add documentation to your project part.
#
# just put add_doc(doc_name) in your directory and off you go
# if you want the main doc to link against this doc you will have to add
# a tagfile variable to doc/devhandbook/CMakeLists.txt add_doc
#
# macro add_doc(doc_name [tagfile] ...)
#	doc_name	- your target will be doc_${doc_name}
#			- this will also be the output folder in apidoc
#			- and the name of the tag-file
#	tagfile		- specify a tagfile to link your dock agains another doc
#			- (don't use the file name but the doc_name .i.e (libyzis)
#	...		- for more tagfiles
#	the .doxy.in file will be sought in the current working directory
#	if it is not found there it will take the default from doc/default.doxy.in

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

endif (ENABLE_DOCUMENTATION)

macro(add_doc _doc_name)
	if (ENABLE_DOCUMENTATION)
		# this is just that it can be accessed in the doxy.in files
		set(doc_name ${_doc_name})
		set(outfolder ${apidir}/${doc_name})

		# check for *.doxy.in in current source dir else take doc/default.doxy.in
		file(GLOB doxyfile "${CMAKE_CURRENT_SOURCE_DIR}/*.doxy.in")
		if( "list(LENGTH ${doxyfile})" GREATER 1 )
			message(SEND_ERROR "there is more then one doxyfile for ${_target}" )
		elseif( "${doxyfile}" STREQUAL "" )
			set(doxyfile ${CMAKE_SOURCE_DIR}/doc/default.doxy.in)
		endif()

		get_filename_component( doxy_file_name ${doxyfile} NAME_WE )
		set( doxy_file_name ${CMAKE_CURRENT_BINARY_DIR}/${doxy_file_name}.doxy)

		# here we parse the tag args
		SET(tag_files "")
		foreach(loop_var ${ARGN})
			SET(tag_files "${tag_files} ${apidir}/${loop_var}/${loop_var}.doxytag=../../${loop_var}/html")
		endforeach()

		# this parses the doxy.in files and replaces @variable@ with
		# the value of the variable
		configure_file( ${doxyfile} ${doxy_file_name} )

		# this is the part that actually generates the doc targets
		add_custom_target(
			doc_${doc_name}
			COMMAND ${DOXYGEN} ${doxy_file_name}
			COMMENT "Build ${doc_name} documentation"
			)

		# here we just make sure this doc is generated then we run
		# "make docs"
		add_dependencies(
			docs doc_${doc_name}
			)

		foreach( loop_var ${ARGN} )
			add_dependencies(
				doc_${doc_name} doc_${loop_var}
				)
		endforeach()

		if ( ${doc_name} STREQUAL "devhandbook" )
			SET( all_doc_links "" )
			foreach( loop_var ${ARGN} )
				SET( all_doc_links "${all_doc_links}    - <a href=\"../../${loop_var}/html/index.html\">${loop_var}</a>\n")
			endforeach()
		endif()
	endif (ENABLE_DOCUMENTATION)
endmacro()
