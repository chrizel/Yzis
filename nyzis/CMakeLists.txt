
project(nyzis)

option(ENABLE_X_IN_NYZIS "with this option nyzis links to X and can access the X copy/paste buffer" OFF)

include_directories(${YZIS_INCLUDES} ${NCURSES_INCLUDE_DIR})

########### next target ###############
set(nyzis_SRCS
   main.cpp
   nysession.cpp
   nyview.cpp
)
set(nyzis_MOC nysession.h)

qt4_wrap_cpp( nyzis_MOC_SRCS ${nyzis_MOC} )
add_executable(nyzis ${nyzis_SRCS} ${nyzis_MOC_SRCS} )
target_link_libraries(nyzis ${NCURSES_LIBRARIES} ${QT_QTCORE_LIBRARY} yzis )

if (ENABLE_X_IN_NYZIS)
	target_link_libraries(nyzis ${NCURSES_LIBRARIES} ${X11_LIBRARIES} ${QT_QTCORE_LIBRARY} yzis )
else (ENABLE_X_IN_NYZIS)
	target_link_libraries(nyzis ${NCURSES_LIBRARIES} ${QT_QTCORE_LIBRARY} yzis )
endif (ENABLE_X_IN_NYZIS)

install(TARGETS nyzis DESTINATION bin)

