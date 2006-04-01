add_subdirectory( pics )
include_directories( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/libyzis ${CMAKE_SOURCE_DIR}/qyzis/ ${CMAKE_SOURCE_DIR}/libyzis/ ${QT_INCLUDES}  )

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

add_definitions("-DQT3_SUPPORT")
QT4_WRAP_CPP(qyzis_MOC_SRCS ${qyzis_MOC})
add_executable(qyzis ${qyzis_SRCS} ${qyzis_MOC_SRCS} )
target_link_libraries(qyzis ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} yzis )
install_targets(/bin qyzis )

########### install files ###############
install_files( ${APPLNK_INSTALL_DIR}/Editors FILES qyzis.desktop )

