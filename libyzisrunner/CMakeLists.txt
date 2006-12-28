
project(libyzisrunner)

include_directories(${YZIS_INCLUDES}) # ${CMAKE_SOURCE_DIR}/libyzis

########### next target ###############

set(libyzisrunner_SRCS 
    ../libyzis/debug.cpp
    ../libyzis/option.cpp
    ../libyzis/internal_options.cpp
    ../libyzis/search.cpp
   main.cpp
)

set(CMAKE_EXE_LINKER_FLAGS 
    "-Wl,-enable-stdcall-fixup,-enable-auto-import,-enable-runtime-pseudo-reloc " 
    ${CMAKE_EXE_LINKER_FLAGS}
    )

if (UNIX OR WIN32)
	add_executable(libyzisrunner ${libyzisrunner_SRCS})
	install(TARGETS libyzisrunner DESTINATION bin)
endif (UNIX OR WIN32)

if (APPLE)
	# Mac OSX Bundle
	set(MACOSX_BUNDLE_BUNDLE_NAME libyzisrunner)
	set(MACOSX_BUNDLE_ICON_FILE libyzisrunner.icns)
	add_executable(libyzisrunner MACOSX_BUNDLE ${libyzisrunner_SRCS} ${libyzisrunner_MOC_SRCS})
	install(TARGETS libyzisrunner DESTINATION ${APPLE_APPLICATION_DIR})
endif (APPLE)

target_link_libraries(libyzisrunner ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} yzis )

