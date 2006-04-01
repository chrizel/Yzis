project(yzis)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_COLOR_MAKEFILE ON)

option(ENABLE_KYZIS "enable kyzis compilation" ON)
option(ENABLE_QYZIS "enable qyzis compilation" ON)
option(ENABLE_NYZIS "enable nyzis compilation" ON)
option(ENABLE_TESTS "enable tests compilation" OFF)

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

if (ENABLE_NYZIS)
	find_package(Ncurses)
	if (NOT NCURSESW_FOUND AND NOT HAVE_NCURSESW)
		message (STATUS "The ncurses library does not support wide-character, nyzis will not be compiled.")
	else (NOT NCURSESW_FOUND AND NOT HAVE_NCURSESW)
		add_subdirectory( nyzis )
	endif (NOT NCURSESW_FOUND AND NOT HAVE_NCURSESW)
endif (ENABLE_NYZIS)


#conditional KDE build
if (KDE4_FOUND)
	add_definitions(${KDE4_DEFINITIONS})
	link_directories(${KDE4_LIB_DIR})
	include_directories( ${KDE4_INCLUDE_DIR} )
	if (ENABLE_KYZIS)
		add_subdirectory( kyzis )
	endif (ENABLE_KYZIS)
	if (ENABLE_TESTS)
		add_subdirectory( tests )
	endif (ENABLE_TESTS)
endif (KDE4_FOUND)

if (ENABLE_QYZIS)
	add_subdirectory( qyzis )
endif (ENABLE_QYZIS)
add_subdirectory( translations )
add_subdirectory( syntax )
add_subdirectory( scripts )

include_directories( ${QT_INCLUDES} )

