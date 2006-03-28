#kde4_header()
include_directories( ${NCURSES_INCLUDE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/libyzis/ ${CMAKE_BINARY_DIR}/libyzis/ ${QT_INCLUDES} )

########### next target ###############
set(nyzis_SRCS viewwidget.cpp factory.cpp main.cpp )
#kde4_automoc(${nyzis_SRCS})
#kde4_add_executable(nyzis ${nyzis_SRCS})
#MOC files
set(nyzis_MOC factory.h)
QT4_WRAP_CPP( nyzis_MOC_SRCS ${nyzis_MOC} )
add_executable(nyzis ${nyzis_SRCS} ${nyzis_MOC_SRCS} )
target_link_libraries(nyzis ${NCURSES_LIBRARIES} ${X11_LIBRARIES} ${QT_QTCORE_LIBRARY} yzis )
install_targets(/bin nyzis )

########### install files ###############
#kde4_footer()
