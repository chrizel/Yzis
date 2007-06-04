project( yzis_kpart )


include_directories( ${YZIS_INCLUDES} ${KDE4_INCLUDE_DIR}
	   ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
	   ${QT_INCLUDES} ${YZIS_INCLUDES}
	   src/
	   )
add_definitions( ${KDE4_DEFINITIONS} )
link_directories( ${KDE4_LIB_DIR} )

set( part_SOURCES
	ktefactory.cpp
	ktedocument.cpp
	kteeditor.cpp
	kteview.cpp
	kyzissession.cpp
	kyzisview.cpp
	kyziscommandwidget.cpp
	kyziscursor.cpp
	kyziseditor.cpp
	   	)

kde4_automoc( ${part_SOURCES} )

kde4_add_plugin(yzis_kpart  ${part_SOURCES})
target_link_libraries(yzis_kpart ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ktexteditor yzis )

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} yzis_kpart )
install_targets( ${PLUGIN_INSTALL_DIR} yzis_kpart )
install_files( ${SERVICES_INSTALL_DIR} FILES  yzis_kpart.desktop )
install_files( ${DATA_INSTALL_DIR}/yzis_kpart FILES  yzis_kpart.rc )
