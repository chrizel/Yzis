
project(qyzis)

include_directories(${YZIS_INCLUDES}) # ${CMAKE_SOURCE_DIR}/libyzis

add_subdirectory( pics )

########### next target ###############

set(qyzis_UI
   qyconfiguredialog.ui
)

set(qyzis_SRCS 
   qycommandline.cpp 
   qyconfiguredialog.cpp
   qycursor.cpp 
   qyedit.cpp 
   qystatusbar.cpp
   qylinenumbers.cpp 
   main.cpp 
   qysession.cpp
   qyview.cpp 
)

set(qyzis_MOC
   qycommandline.h
   qyconfiguredialog.h
   qycursor.h
   qyedit.h
   qylinenumbers.h
   qysession.h
   qyview.h
)

qt4_wrap_ui(qyzis_SRCS ${qyzis_UI})
qt4_wrap_cpp(qyzis_MOC_SRCS ${qyzis_MOC})
qt4_add_resources(qyzis_RESOURCES qyzis.qrc)

if (MINGW)
    # resource compilation for MinGW
    ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qyzis_ico.obj
                        COMMAND windres.exe 
                            -I${CMAKE_CURRENT_SOURCE_DIR} 
                            -o ${CMAKE_CURRENT_BINARY_DIR}/qyzis_ico.obj
                            < ${CMAKE_CURRENT_SOURCE_DIR}/qyzis_ico.rc
                        )
    SET( qyzis_SRCS ${qyzis_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/qyzis_ico.obj )
endif(MINGW)

if (WIN32)
    # common to windows msvc/mingw32
    add_executable(qyzis ${qyzis_SRCS} ${qyzis_MOC_SRCS} ${qyzis_RESOURCES} ${qyzis_UI_SRCS} )
    install(TARGETS qyzis DESTINATION bin)
    if (MINGW)
        # specific to mingw32
        set_target_properties(qyzis PROPERTIES LINK_FLAGS "-mwindows")
    endif (MINGW)
endif (WIN32)

if (UNIX)
    add_executable(qyzis ${qyzis_SRCS} ${qyzis_MOC_SRCS} ${qyzis_RESOURCES} ${qyzis_UI_SRCS} )
    install(TARGETS qyzis DESTINATION bin)
endif (UNIX)

if(APPLE)
	# Mac OSX Bundle
	set(MACOSX_BUNDLE_BUNDLE_NAME QYzis)
	set(MACOSX_BUNDLE_ICON_FILE qyzis.icns)
	add_executable(qyzis MACOSX_BUNDLE ${qyzis_SRCS} ${qyzis_MOC_SRCS})
	install(TARGETS qyzis DESTINATION ${APPLE_APPLICATION_DIR})
endif (APPLE)

target_link_libraries(qyzis ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} yzis )

########### install files ###############
install( FILES qyzis.desktop DESTINATION ${APPLNK_INSTALL_DIR}/Editors )

