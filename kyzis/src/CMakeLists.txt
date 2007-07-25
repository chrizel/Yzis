
add_subdirectory( newui )

include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
${CMAKE_SOURCE_DIR}/kyzis/src/newui/ ${CMAKE_SOURCE_DIR}/libyzis/
${CMAKE_BINARY_DIR}/libyzis/ ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}
${YZIS_INCLUDES} )


########### next target ###############
add_definitions("-DQT3_SUPPORT")
SET(internkyzis_SRCS
   commandwidget.cpp
   cursor.cpp
   editor.cpp
   factory.cpp
   konsole.cpp
   ktexteditoriface.cpp
   kyzisbuffer.cpp
   kyziscodecompletion.cpp
   kyzis.cpp
   viewwidget.cpp
)

#KDE4_ADD_DCOP_SKELS (internkyzis_SRCS kyzis.h)
KDE4_ADD_KCFG_FILES (internkyzis_SRCS settings.kcfgc)

ADD_LIBRARY(internkyzis STATIC ${internkyzis_SRCS} )

########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES  kyzis.kcfg )

