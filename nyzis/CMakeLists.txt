
project(nyzis)

include_directories(${YZIS_INCLUDES} ${NCURSES_INCLUDE_DIR})

########### next target ###############
set(nyzis_SRCS
   nsession.cpp
   main.cpp
   viewwidget.cpp
)
set(nyzis_MOC nsession.h)

qt4_wrap_cpp( nyzis_MOC_SRCS ${nyzis_MOC} )
add_executable(nyzis ${nyzis_SRCS} ${nyzis_MOC_SRCS} )
target_link_libraries(nyzis ${NCURSES_LIBRARIES} ${X11_LIBRARIES} ${QT_QTCORE_LIBRARY} yzis )

install(TARGETS nyzis DESTINATION bin)

