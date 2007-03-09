# - Try to find the LibLuaLib50 
# Once done this will define
#
#  LIBLUALIB50_FOUND - system has the LibLuaLib50
#  LIBLUALIB50_INCLUDE_DIR - the LibLuaLib50 include directory
#  LIBLUALIB50_LIBRARIES - The libraries needed to use LibLuaLib50
# under Windows this also checks in the GNUWIN32 directory, so make
# sure that the GNUWIN32 directory gets found if you use the GNUWIN32 version of PCRE
# under UNIX pkgconfig among others pkg-config is used to find the directories


INCLUDE(UsePkgConfig)

message(STATUS "Looking for liblualib(50)")

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
PKGCONFIG(lualib50 _libLualib50IncDir _libLualib50LinkDir _libLualib50LinkFlags _libLualib50Cflags)
#retry lua.pc for gentoo ...
PKGCONFIG(lua _libLualibIncDir _libLualibLinkDir _libLualibLinkFlags _libLualibCflags)

set(LIBLUALIB50_DEFINITIONS ${_libLualib50Cflags} ${_libLualibCflags} )

# under windows, try to find the base gnuwin32 directory, do nothing under UNIX
FIND_PACKAGE(GNUWIN32)

FIND_PATH(LIBLUALIB50_INCLUDE_DIR lualib.h
   ${WITH_LUA_HOME}/include
   ${_libLualib50IncDir}/lua50
   ${_libLualibIncDir}
   /usr/include/lua50
   /usr/local/include/lua50
   ${GNUWIN32_DIR}/include
)

FIND_LIBRARY(LIBLUALIB50_LIBRARIES NAMES lualib50 lualib lua
   PATHS
   ${WITH_LUA_HOME}/lib
   ${_libLualib50LinkDir}
   ${_libLualibLinkDir}
   /usr/lib
   /usr/local/lib
   ${GNUWIN32_DIR}/lib
)


if (LIBLUALIB50_INCLUDE_DIR AND LIBLUALIB50_LIBRARIES)
   set(LIBLUALIB50_FOUND TRUE)
endif (LIBLUALIB50_INCLUDE_DIR AND LIBLUALIB50_LIBRARIES)


if (LIBLUALIB50_FOUND)
   if (NOT LibLuaLib50_FIND_QUIETLY)
      message(STATUS "Looking for liblualib(50) - found: ${LIBLUALIB50_LIBRARIES}")
   endif (NOT LibLuaLib50_FIND_QUIETLY)
else (LIBLUALIB50_FOUND)
   if (LibLuaLib50_FIND_REQUIRED)
      message(FATAL_ERROR "Looking for liblualib(50) - not found")
   endif (LibLuaLib50_FIND_REQUIRED)
endif (LIBLUALIB50_FOUND)

MARK_AS_ADVANCED(LIBLUALIB50_INCLUDE_DIR LIBLUALIB50_LIBRARIES)
