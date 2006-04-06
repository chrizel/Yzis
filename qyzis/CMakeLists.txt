add_subdirectory( pics )
include_directories( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/libyzis ${CMAKE_SOURCE_DIR}/qyzis/ ${CMAKE_SOURCE_DIR}/libyzis/ ${QT_INCLUDES}  ${YZIS_INCLUDES} )

########### next target ###############

set(qyzis_SRCS 
   main.cpp 
   viewwidget.cpp 
   factory.cpp 
   editor.cpp 
   commandwidget.cpp 
   cursor.cpp 
   qyzis.cpp )

set(qyzis_MOC
   commandwidget.h
   cursor.h
   editor.h
   factory.h
   qyzis.h
   viewwidget.h
)

QT4_WRAP_CPP(qyzis_MOC_SRCS ${qyzis_MOC})
add_executable(qyzis MACOSX_BUNDLE ${qyzis_SRCS} ${qyzis_MOC_SRCS} )
target_link_libraries(qyzis ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} yzis )
if (NOT APPLE)
	install_targets(/bin qyzis)
else (NOT APPLE)
	install(TARGETS qyzis DESTINATION ${APPLE_APPLICATION_DIR})
endif (NOT APPLE)

########### install files ###############
install_files( ${APPLNK_INSTALL_DIR}/Editors FILES qyzis.desktop )

