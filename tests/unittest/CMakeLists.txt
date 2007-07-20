
# sources
set(yzis_unittest_SRCS 
    main.cpp 
    testDebug.cpp 
    testColor.cpp 
    testResource.cpp 
    ../../libyzis/debug.cpp 
    ../../libyzis/color.cpp 
    ../../libyzis/resourcemgr.cpp 
)

# files needing moc
set(yzis_unittest_MOCS 
   testDebug.h
   testColor.h
   testResource.h
)

qt4_wrap_cpp(yzis_unittest_MOC_SRCS ${yzis_unittest_MOCS})

add_executable(yzis_unittest ${yzis_unittest_SRCS} ${yzis_unittest_MOC_SRCS} )

include_directories( 
    ${CMAKE_BINARY_DIR} 
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_BINARY_DIR}/libyzis/ 
    ${CMAKE_SOURCE_DIR}/libyzis/ 
    ${QT_INCLUDES}  
    ${LIBLUA_INCLUDE_DIR}
    ${YZIS_INCLUDES}
    )

target_link_libraries(yzis_unittest 
    ${QT_QTTEST_LIBRARY} 
    ${QT_QTCORE_LIBRARY}
    ${LIBLUA_LIBRARIES}
    ${MAGIC_LIBRARIES} 
    ${GETTEXT_LIBRARIES}
    yzis
    )

install(TARGETS yzis_unittest DESTINATION bin)

add_test(yzis_unittest yzis_unittest)
