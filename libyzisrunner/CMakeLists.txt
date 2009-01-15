
project(libyzisrunner)

include_directories(${YZIS_INCLUDES}) # ${CMAKE_SOURCE_DIR}/libyzis

########### next target ###############

set(libyzisrunner_SRCS 
    NoGuiSession.cpp
    NoGuiView.cpp
    ../libyzis/debug.cpp
    ../libyzis/option.cpp
    ../libyzis/internal_options.cpp
    ../libyzis/search.cpp
   main.cpp
)

set(libyzisrunner_MOC
    NoGuiSession.h
)

qt4_wrap_cpp(libyzisrunner_MOC_SRCS ${libyzisrunner_MOC})

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS 
        "-Wl,-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc " 
        ${CMAKE_EXE_LINKER_FLAGS}
        )
endif(MINGW)

if (APPLE)
	# Mac OSX Bundle
	set(MACOSX_BUNDLE_BUNDLE_NAME libyzisrunner)
	set(MACOSX_BUNDLE_ICON_FILE libyzisrunner.icns)
	add_executable(libyzisrunner MACOSX_BUNDLE ${libyzisrunner_SRCS} ${libyzisrunner_MOC_SRCS})
	install(TARGETS libyzisrunner DESTINATION ${APPLE_APPLICATION_DIR})
else (APPLE)
	add_executable(libyzisrunner ${libyzisrunner_SRCS} ${libyzisrunner_MOC_SRCS} )
	install(TARGETS libyzisrunner DESTINATION bin)
endif (APPLE)

target_link_libraries(libyzisrunner ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} yzis )

