
include_directories( 
    ${CMAKE_BINARY_DIR} 
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_BINARY_DIR}/libyzis/ 
    ${CMAKE_SOURCE_DIR}/libyzis/ 
    ${QT_INCLUDES}  
    )

# sources
set(yzistest_SRCS 
    main.cpp 
    testDebug.cpp 
    testColor.cpp 
    ../../libyzis/debug.cpp 
    ../../libyzis/color.cpp 
    )

# files needing moc
set(yzistest_MOCS 
   testDebug.h
   testColor.h
)

qt4_wrap_cpp(yzistest_MOC_SRCS ${yzistest_MOCS})

add_executable(yzistest ${yzistest_SRCS} ${yzistest_MOC_SRCS} )

target_link_libraries(yzistest 
    ${QT_QTTEST_LIBRARY} 
    ${QT_QTCORE_LIBRARY}
    )

install(TARGETS yzistest DESTINATION bin)

