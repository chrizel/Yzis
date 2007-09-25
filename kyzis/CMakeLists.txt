project( kyzis )


include_directories( ${YZIS_INCLUDES} ${KDE4_INCLUDE_DIR}
                     ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
                     ${QT_INCLUDES} ${YZIS_INCLUDES}
                   )
add_definitions( ${KDE4_DEFINITIONS} )
link_directories( ${KDE4_LIB_DIR} )

set( kyzis_SOURCES
     main.h
     main.cpp
    )

kde4_add_executable(kyzis ${kyzis_SOURCES})
target_link_libraries(kyzis ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} yzis ktexteditor)
