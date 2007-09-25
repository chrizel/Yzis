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
	kysession.cpp
	kyview.cpp
	kycommand.cpp
	kycursor.cpp
	kyeditor.cpp
	kyinfobar.cpp
	   	)

kde4_add_plugin(yzispart ${part_SOURCES})
target_link_libraries(yzispart ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ktexteditor yzis )

install( TARGETS yzispart DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES yzispart.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES yzispart.rc DESTINATION ${DATA_INSTALL_DIR}/yzispart )
