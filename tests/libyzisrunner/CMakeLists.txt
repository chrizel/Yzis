kde4_header()

include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/libyzis/ ${CMAKE_SOURCE_DIR}/libyzis/ ${QT_INCLUDES}  )

########### next target ###############

set(libyzisrunner_SRCS main.cpp)
kde4_automoc(${libyzisrunner_SRCS})
kde4_add_executable(libyzisrunner ${libyzisrunner_SRCS})
target_link_libraries(libyzisrunner yzis ${X11_LIBRARIES} ${QT_QTCORE_LIBRARY} )
install_targets(/bin libyzisrunner )


########### install files ###############
kde4_footer()

