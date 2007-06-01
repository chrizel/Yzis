project( yzis_kpart )


include_directories( ${YZIS_INCLUDES} ${KDE4_INCLUDE_DIR}
	   ${CMAKE_SOURCE_DIR}/libyzis/ ${CMAKE_BINARY_DIR}/libyzis
	   ${QT_INCLUDES} ${YZIS_INCLUDES}
	   src/
	   )
add_definitions( ${KDE4_DEFINITIONS} )
link_directories( ${KDE4_LIB_DIR} )

set( part_SOURCES
	kyzisfactory.cpp
	kyzisdocument.cpp
	kyziseditor.cpp
	kyzisview.cpp
	   	)

kde4_automoc( ${part_SOURCES} )
kde4_add_plugin(yzis_kpart  ${part_SOURCES})
target_link_libraries(yzis_kpart ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ktexteditor yzis )

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} yzis_kpart )
install_targets( ${PLUGIN_INSTALL_DIR} yzis_kpart )
install_files( ${SERVICES_INSTALL_DIR} FILES  yzis_kpart.desktop )
install_files( ${DATA_INSTALL_DIR}/yzis_kpart FILES  yzis_kpart.rc )
