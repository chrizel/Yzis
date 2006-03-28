kde4_header()

include_directories( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/libyzis/ ${CMAKE_SOURCE_DIR}/libyzis/ ${QT_INCLUDES}  )


########### next target ###############

set(yzistest_SRCS main.cpp TSession.cpp )
kde4_automoc(${yzistest_SRCS})
kde4_add_executable(yzistest ${yzistest_SRCS})
target_link_libraries(yzistest yzis ${QT_QTTEST_LIBRARY} ${X11_LIBRARIES} )
install_targets(/bin yzistest )


########### install files ###############
kde4_footer()

