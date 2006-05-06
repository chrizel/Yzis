
project(qyzis)

include_directories(${YZIS_INCLUDES}) # ${CMAKE_SOURCE_DIR}/libyzis

add_subdirectory( pics )

########### next target ###############

set(qyzis_SRCS 
   commandwidget.cpp 
   cursor.cpp 
   editor.cpp 
   factory.cpp 
   main.cpp 
   qyzis.cpp
   viewwidget.cpp 
)

set(qyzis_MOC
   commandwidget.h
   cursor.h
   editor.h
   factory.h
   qyzis.h
   viewwidget.h
)

QT4_WRAP_CPP(qyzis_MOC_SRCS ${qyzis_MOC})
if (NOT APPLE)
	add_executable(qyzis ${qyzis_SRCS} ${qyzis_MOC_SRCS} )
	install_targets(/bin qyzis)
else (NOT APPLE)
	# Mac OSX Bundle
	set(MACOSX_BUNDLE_BUNDLE_NAME QYzis)
	set(MACOSX_BUNDLE_ICON_FILE qyzis.icns)
	add_executable(qyzis MACOSX_BUNDLE ${qyzis_SRCS} ${qyzis_MOC_SRCS} )
	install(TARGETS qyzis DESTINATION ${APPLE_APPLICATION_DIR})
endif (NOT APPLE)
target_link_libraries(qyzis ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} yzis )

########### install files ###############
install_files( ${APPLNK_INSTALL_DIR}/Editors FILES qyzis.desktop )

