project( kpart_yzis )

include_directories( ${YZIS_INCLUDES} ${KDE4_INCLUDE_DIR}
	   ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
	   ${QT_INCLUDES}
)
add_definitions( ${KDE4_DEFINITIONS} )
link_directories( ${KDE4_LIB_DIR} )

set(qyzis_SRCS 
   ../qyzis/qycommandline.cpp 
   ../qyzis/qyconfiguredialog.cpp
   ../qyzis/qycursor.cpp 
   ../qyzis/qyedit.cpp 
   ../qyzis/qystatusbar.cpp
   ../qyzis/qylinenumbers.cpp 
   ../qyzis/qysession.cpp
   ../qyzis/qyview.cpp 
)
set(qyzis_UI
   ../qyzis/qyconfiguredialog.ui
)
qt4_wrap_ui(qyzis_SRCS ${qyzis_UI})

set( part_SOURCES
	${qyzis_SRCS}
	ktefactory.cpp
	ktedocument.cpp
	kteeditor.cpp
	kteview.cpp
	kysession.cpp
	kyview.cpp
	kyeditor.cpp
)


kde4_add_plugin(yzispart ${part_SOURCES})
target_link_libraries(yzispart ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KTEXTEDITOR_LIBS} yzis )

install( TARGETS yzispart DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES yzispart.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES yzispart.rc DESTINATION ${DATA_INSTALL_DIR}/yzispart )
