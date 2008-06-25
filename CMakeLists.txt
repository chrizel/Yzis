SET( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )

project(yzis)

enable_testing()
#include(CTest)

# Always include srcdir and builddir in include path
# This saves typing ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY} in about every subdir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# put the include dirs which are in the source or build tree
# before all other include dirs, so the headers in the sources
# are prefered over the already installed ones
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# Use colored output
set(CMAKE_COLOR_MAKEFILE ON)


if (WIN32)
    cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
else(WIN32)
    cmake_minimum_required(VERSION 2.4.8 FATAL_ERROR)
endif(WIN32)

# yzis stores the cmake modules in cmake/modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(CheckCXXCompilerFlag)
include(ConfigureChecks.cmake)
include(ConvenienceLibs.cmake)
include(MacroAdditionalCleanFiles)

set( CCMAKE_CXX_FLAGS_RELEASE "-O3" )
set( CCMAKE_C_FLAGS_RELEASE "-O3" )

set( CMAKE_CXX_FLAGS_DEBUG "-g3 -Wall -DDEBUG" )
set( CMAKE_C_FLAGS_DEBUG "-g3 -Wall -DDEBUG" )

########################## options for compilation
#
if (WIN32)
    option(ENABLE_NYZIS "enable nyzis compilation" OFF)
else(WIN32)
    option(ENABLE_NYZIS "enable nyzis compilation" ON)
endif(WIN32)

option(ENABLE_QYZIS "enable qyzis compilation" ON)
option(ENABLE_KYZIS "enable kyzis compilation" OFF)
option(ENABLE_KPART_YZIS "enable yzis_kpart compilation" OFF)
option(ENABLE_LIBYZISRUNNER "enable libyzisrunner compilation" ON)
option(ENABLE_TESTS "enable tests compilation" ON)

option(ENABLE_DOCUMENTATION "Allow doxygen documentation generation with 'make docs'" ON)
option(GENERATE_DOC "Generate doxygen documentation during build" OFF)

option(ENABLE_SAFE_MODE "prevent yzis from crashing when some bugs occured" ON)

########################## platform specific stuff
#
if (WIN32)
    add_definitions("-DYZIS_WIN32_GCC")
    find_package(GNUWIN32)
    link_libraries(imm32 winmm)  # missing by default :/
endif(WIN32)

if(WIN32 AND CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-static-libgcc ${CMAKE_CXX_FLAGS}")
  link_directories(${CMAKE_BINARY_DIR}/staticlibs) # override dynamic libs
endif(WIN32 AND CMAKE_COMPILER_IS_GNUCXX)

find_package(Magic REQUIRED)
find_package(Gettext REQUIRED)

if (UNIX)
    find_package(X11 REQUIRED)
endif (UNIX)


if (APPLE) # XXX: workaround intl for Mac OSX
	set(GETTEXT_LIBRARIES ${LIBINTL_LIBRARY})
endif(APPLE)

### lua
find_package(LibLua51 REQUIRED)
#if (NOT WIN32)
#    find_package(LibLuaLib51 REQUIRED)
#endif (NOT WIN32)
set(LIBLUA_INCLUDE_DIR ${LIBLUA51_INCLUDE_DIR})
set(LIBLUA_LIBRARIES ${LIBLUA51_LIBRARIES})

### Qt
if (ENABLE_QYZIS)
    # QYzis requires Qt 4.2
    set(QT_MIN_VERSION "4.2.0")
else (ENABLE_QYZIS)
    set(QT_MIN_VERSION "4.1.0")
endif (ENABLE_QYZIS)

find_package(Qt4 REQUIRED)

if (NOT QT_QTTEST_FOUND)
	MESSAGE(FATAL_ERROR "Could not find qt-test library")
endif(NOT QT_QTTEST_FOUND)

if (NOT QT_QTGUI_FOUND)
	MESSAGE(FATAL_ERROR "Could not find qt-gui library")
endif(NOT QT_QTGUI_FOUND)

if (NOT QT_QTXML_FOUND)
	MESSAGE(FATAL_ERROR "Could not find qt-xml library")
endif (NOT QT_QTXML_FOUND)


include( FixQt4DebugBuild )

# for syntax files
if (NOT WIN32)
find_package(Perl REQUIRED)
endif(NOT WIN32)

# optional packages dependencies :
if (ENABLE_KYZIS OR ENABLE_KPART_YZIS)
	find_package(KDE4 REQUIRED)
endif (ENABLE_KYZIS OR ENABLE_KPART_YZIS)

if (ENABLE_NYZIS)
	find_package(Ncurses)
	if (NOT NCURSES_FOUND)
		message(FATAL_ERROR "Could not find ncurses library.")
	endif (NOT NCURSES_FOUND)

	if (NOT NCURSESW_FOUND AND NOT HAVE_NCURSESW)
		message(FATAL_ERROR "The ncurses library does not support wide-character.")
	endif (NOT NCURSESW_FOUND AND NOT HAVE_NCURSESW)
endif (ENABLE_NYZIS)

#
# define some variables
#
if (NOT APPLNK_INSTALL_DIR)
	set(APPLNK_INSTALL_DIR share/applnk)
endif (NOT APPLNK_INSTALL_DIR)

if (NOT ICON_INSTALL_DIR)
	set(ICON_INSTALL_DIR share/icons)
endif (NOT ICON_INSTALL_DIR)

# XXX how deal with macosx bundles?
if (APPLE)
	if (NOT APPLE_APPLICATION_DIR)
		set(APPLE_APPLICATION_DIR /Applications)
	endif (NOT APPLE_APPLICATION_DIR)
endif (APPLE)


############################# build instructions
#

# visibility support
if (CMAKE_COMPILER_IS_GNUCXX)
	check_cxx_compiler_flag(-fvisibility=hidden __YZIS_HAVE_GCC_VISIBILITY)
	if (__YZIS_HAVE_GCC_VISIBILITY)
	     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
	endif (__YZIS_HAVE_GCC_VISIBILITY)
endif (CMAKE_COMPILER_IS_GNUCXX)

# libyzis dependencies includes
set(YZIS_INCLUDES ${QT_INCLUDES} ${LIBLUA_INCLUDE_DIR} ${MAGIC_INCLUDE_DIR})
# libyzis includes
set(YZIS_INCLUDES ${YZIS_INCLUDES} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/libyzis ${CMAKE_SOURCE_DIR}/libyzis)

add_definitions(${QT_DEFINITIONS})

if (ENABLE_SAFE_MODE)
	add_definitions("-DSAFE_MODE")
endif (ENABLE_SAFE_MODE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/yzismacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/yzismacros.h )

# macro_additional_clean_files(
#	${CMAKE_CURRENT_BINARY_DIR}/config.h
#	${CMAKE_CURRENT_BINARY_DIR}/yzismacros.h
#)


####################### subdirectories to compile
#
add_subdirectory( libyzis )

if (ENABLE_LIBYZISRUNNER)
	add_subdirectory( libyzisrunner )
endif (ENABLE_LIBYZISRUNNER)

if (ENABLE_NYZIS)
	add_subdirectory( nyzis )
endif (ENABLE_NYZIS)

if (ENABLE_QYZIS)
	add_subdirectory( qyzis )
endif (ENABLE_QYZIS)

if (ENABLE_KYZIS)
	add_subdirectory( kyzis )
endif (ENABLE_KYZIS)

if (ENABLE_KPART_YZIS)
	add_subdirectory( kpart_yzis )
endif (ENABLE_KPART_YZIS)

if (ENABLE_TESTS)
	add_subdirectory( tests )
endif (ENABLE_TESTS)

add_subdirectory( doc )
#add_subdirectory( translations )
add_subdirectory( syntax )
add_subdirectory( scripts )

