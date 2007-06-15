project( kpart_yzis )


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
	kyzisinfobar.cpp
	   	)

kde4_automoc( ${part_SOURCES} )

kde4_add_plugin(yzispart ${part_SOURCES})
target_link_libraries(yzispart ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ktexteditor yzis )

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} yzispart )
install_targets( ${PLUGIN_INSTALL_DIR} yzispart )
install_files( ${SERVICES_INSTALL_DIR} FILES  yzispart.desktop )
install_files( ${DATA_INSTALL_DIR}/yzispart FILES  yzispart.rc )
