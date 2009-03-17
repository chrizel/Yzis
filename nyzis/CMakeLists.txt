
project(nyzis)

option(ENABLE_X_IN_NYZIS "with this option nyzis links to X and can access the X copy/paste buffer" OFF)

include_directories(${YZIS_INCLUDES} ${NCURSES_INCLUDE_DIR})

########### next target ###############
set(nyzis_SRCS
   main.cpp
   nysession.cpp
   nyview.cpp
   nystatusbar.cpp
)

qt4_automoc(${nyzis_SRCS})
add_executable(nyzis ${nyzis_SRCS})
target_link_libraries(nyzis ${NCURSES_LIBRARIES} ${QT_QTCORE_LIBRARY} yzis )

if (ENABLE_X_IN_NYZIS)
	target_link_libraries(nyzis ${NCURSES_LIBRARIES} ${X11_LIBRARIES} ${QT_QTCORE_LIBRARY} yzis )
else (ENABLE_X_IN_NYZIS)
	target_link_libraries(nyzis ${NCURSES_LIBRARIES} ${QT_QTCORE_LIBRARY} yzis )
endif (ENABLE_X_IN_NYZIS)

install(TARGETS nyzis DESTINATION bin)

add_doc(nyzis libyzis)
