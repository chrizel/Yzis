project(yzis)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_COLOR_MAKEFILE ON)

#KDE4 non-required
find_package(KDE4)
#but we still need qt4 ;)
if (NOT KDE4_FOUND)
	find_package(Qt4 REQUIRED)
	include(MacroAdditionalCleanFiles)
endif (NOT KDE4_FOUND)

find_package(Perl REQUIRED)

if (NOT APPLNK_INSTALL_DIR)
	set(APPLNK_INSTALL_DIR /share/applnk)
endif (NOT APPLNK_INSTALL_DIR)

if (NOT ICON_INSTALL_DIR)
	set(ICON_INSTALL_DIR share/icons)
endif (NOT ICON_INSTALL_DIR)

add_definitions(${QT_DEFINITIONS})

include(ConvenienceLibs.cmake)
include(ConfigureChecks.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/yzismacros.h.in ${CMAKE_CURRENT_BINARY_DIR}/yzismacros.h )
macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/config.h ${CMAKE_CURRENT_BINARY_DIR}/yzismacros.h )

add_subdirectory( libyzis )

find_package(Ncurses)
if (NOT NCURSESW_FOUND AND NOT HAVE_NCURSESW)
	message (STATUS "The ncurses library does not support wide-character, nyzis will not be compiled.")
else (NOT NCURSESW_FOUND AND NOT HAVE_NCURSESW)
	add_subdirectory( nyzis )
endif (NOT NCURSESW_FOUND AND NOT HAVE_NCURSESW)


#conditional KDE build
if (KDE4_FOUND)
	add_definitions(${KDE4_DEFINITIONS})
	link_directories(${KDE4_LIB_DIR})
	include_directories( ${KDE4_INCLUDE_DIR} )
	add_subdirectory( kyzis )
	add_subdirectory( tests )
endif (KDE4_FOUND)

add_subdirectory( translations )
add_subdirectory( qyzis )
add_subdirectory( syntax )
add_subdirectory( scripts )

include_directories( ${QT_INCLUDES} )

