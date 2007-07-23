# - Try to find the LibLuaLib51 
# Once done this will define
#
#  LIBLUALIB51_FOUND - system has the LibLuaLib51
#  LIBLUALIB51_INCLUDE_DIR - the LibLuaLib51 include directory
#  LIBLUALIB51_LIBRARIES - The libraries needed to use LibLuaLib51
# under Windows this also checks in the GNUWIN32 directory, so make
# sure that the GNUWIN32 directory gets found if you use the GNUWIN32 version of PCRE
# under UNIX pkgconfig among others pkg-config is used to find the directories


INCLUDE(UsePkgConfig)

message(STATUS "Looking for liblualib(51)")

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
PKGCONFIG(lualib51 _libLualib51IncDir _libLualib51LinkDir _libLualib51LinkFlags _libLualib51Cflags)
#retry lua.pc for gentoo ...
PKGCONFIG(lua _libLualibIncDir _libLualibLinkDir _libLualibLinkFlags _libLualibCflags)

set(LIBLUALIB51_DEFINITIONS ${_libLualib51Cflags} ${_libLualibCflags} )

# under windows, try to find the base gnuwin32 directory, do nothing under UNIX
FIND_PACKAGE(GNUWIN32)

FIND_PATH(LIBLUALIB51_INCLUDE_DIR lualib.h
   ${WITH_LUA_HOME}/include
   ${_libLualib51IncDir}/lua51
   ${_libLualibIncDir}
   /usr/include/lua5.1
   /usr/include/lua51
   /usr/local/include/lua51
   ${GNUWIN32_DIR}/include
)

FIND_LIBRARY(LIBLUALIB51_LIBRARIES NAMES lualib51 lualib lua
   PATHS
   ${WITH_LUA_HOME}/lib
   ${_libLualib51LinkDir}
   ${_libLualibLinkDir}
   /usr/lib
   /usr/local/lib
   ${GNUWIN32_DIR}/lib
)


if (LIBLUALIB51_INCLUDE_DIR AND LIBLUALIB51_LIBRARIES)
   set(LIBLUALIB51_FOUND TRUE)
endif (LIBLUALIB51_INCLUDE_DIR AND LIBLUALIB51_LIBRARIES)


if (LIBLUALIB51_FOUND)
   if (NOT LibLuaLib51_FIND_QUIETLY)
      message(STATUS "Looking for liblualib(51) - found: ${LIBLUALIB51_LIBRARIES}")
   endif (NOT LibLuaLib51_FIND_QUIETLY)
else (LIBLUALIB51_FOUND)
   if (LibLuaLib51_FIND_REQUIRED)
      message(FATAL_ERROR "Looking for liblualib(51) - not found")
   endif (LibLuaLib51_FIND_REQUIRED)
endif (LIBLUALIB51_FOUND)

MARK_AS_ADVANCED(LIBLUALIB51_INCLUDE_DIR LIBLUALIB51_LIBRARIES)

