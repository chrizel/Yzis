project(yzis)

#enable_testing()
#include(CTest)

# Always include srcdir and builddir in include path
# This saves typing ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY} in about every subdir
# since cmake 2.4.0
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# put the include dirs which are in the source or build tree
# before all other include dirs, so the headers in the sources
# are prefered over the already installed ones
# since cmake 2.4.1
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# Use colored output
# since cmake 2.4.0
set(CMAKE_COLOR_MAKEFILE ON)

cmake_minimum_required(VERSION 2.4.1 FATAL_ERROR)

set(CMAKE_COLOR_MAKEFILE ON)
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
    option(ENABLE_QYZIS "enable qyzis compilation" ON)
    option(ENABLE_NYZIS "enable nyzis compilation" OFF)
else(WIN32)
    option(ENABLE_QYZIS "enable qyzis compilation" ON)
    option(ENABLE_NYZIS "enable nyzis compilation" ON)
endif(WIN32)

option(ENABLE_KYZIS "enable kyzis compilation" OFF)
option(ENABLE_LIBYZISRUNNER "enable libyzisrunner compilation" ON)
option(ENABLE_TESTS "enable tests compilation" ON)
option(ENABLE_DOCUMENTATION "generate doxygen documentation" OFF)

option(ENABLE_SAFE_MODE "prevent yzis from crashing when some bugs occured" ON)

########################## platform specific stuff
#
if (WIN32)
	add_definitions("-DYZIS_WIN32_GCC")
else(WIN32)
    find_package(Magic REQUIRED)
    find_package(Gettext REQUIRED)
endif(WIN32)

if (APPLE) # XXX: workaround intl for Mac OSX
	set(GETTEXT_LIBRARIES ${LIBINTL_LIBRARY})
endif(APPLE)

### lua
find_package(LibLua50 REQUIRED)
if (NOT WIN32)
    find_package(LibLuaLib50 REQUIRED)
endif (NOT WIN32)
set(LIBLUA_INCLUDE_DIR ${LIBLUA50_INCLUDE_DIR} ${LIBLUALIB50_INCLUDE_DIR} )
set(LIBLUA_LIBRARIES ${LIBLUA50_LIBRARIES} ${LIBLUALIB50_LIBRARIES})


### Qt
if (ENABLE_QYZIS)
    # QYzis requires Qt 4.2
    set(QT_MIN_VERSION "4.2.0")
else (ENABLE_QYZIS)
    set(QT_MIN_VERSION "4.1.0")
endif (ENABLE_QYZIS)
find_package(Qt4 REQUIRED)
include( FixQt4DebugBuild )

# for syntax files
find_package(Perl REQUIRED)

# optional packages dependencies :
if (ENABLE_KYZIS)
	find_package(KDE4 REQUIRED)
endif (ENABLE_KYZIS)

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
	set(APPLNK_INSTALL_DIR /share/applnk)
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
macro_additional_clean_files(
	${CMAKE_CURRENT_BINARY_DIR}/config.h
	${CMAKE_CURRENT_BINARY_DIR}/yzismacros.h
)


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
if (ENABLE_TESTS)
	add_subdirectory( tests )
endif (ENABLE_TESTS)
if (ENABLE_DOCUMENTATION)
	add_subdirectory( doc )
endif (ENABLE_DOCUMENTATION)
add_subdirectory( translations )
add_subdirectory( syntax )
add_subdirectory( scripts )

