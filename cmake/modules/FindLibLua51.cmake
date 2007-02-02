# - Try to find the LibLua51 
# Once done this will define
#
#  LIBLUA51_FOUND - system has the LibLua51
#  LIBLUA51_INCLUDE_DIR - the LibLua51 include directory
#  LIBLUA51_LIBRARIES - The libraries needed to use LibLua51
# under Windows this also checks in the GNUWIN32 directory, so make
# sure that the GNUWIN32 directory gets found if you use the GNUWIN32 version of PCRE
# under UNIX pkgconfig among others pkg-config is used to find the directories


INCLUDE(UsePkgConfig)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
#debian
PKGCONFIG(lua5.1 _libLua51IncDir _libLua51LinkDir _libLua51LinkFlags _libLua51Cflags)
#gentoo
PKGCONFIG(lua _libLuaIncDir _libLuaLinkDir _libLuaLinkFlags _libLuaCflags)
#any other distrib to blame for using common naming ?
#

set(LIBLUA51_DEFINITIONS ${_libLua51Cflags} ${_libLuaCflags} )

# under windows, try to find the base gnuwin32 directory, do nothing under UNIX
FIND_PACKAGE(GNUWIN32)

FIND_PATH(LIBLUA51_INCLUDE_DIR lua.h
   ${_libLua51IncDir}/lua
   ${_libLuaIncDir}
   /usr/include/lua5.1
   /usr/local/include/lua5.1
   ${GNUWIN32_DIR}/include
)

FIND_LIBRARY(LIBLUA51_LIBRARIES NAMES lua5.1 lua
   PATHS
   ${_libLua51LinkDir}
   ${_libLuaLinkDir}
   /usr/lib
   /usr/local/lib
   ${GNUWIN32_DIR}/lib
)


if (LIBLUA51_INCLUDE_DIR AND LIBLUA51_LIBRARIES)
   set(LIBLUA51_FOUND TRUE)
endif (LIBLUA51_INCLUDE_DIR AND LIBLUA51_LIBRARIES)


if (LIBLUA51_FOUND)
   if (NOT LibLua51_FIND_QUIETLY)
      message(STATUS "Found liblua(5.1): ${LIBLUA51_LIBRARIES}")
   endif (NOT LibLua51_FIND_QUIETLY)
else (LIBLUA51_FOUND)
   if (LibLua51_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find liblua(5.1)")
   endif (LibLua51_FIND_REQUIRED)
endif (LIBLUA51_FOUND)

MARK_AS_ADVANCED(LIBLUA51_INCLUDE_DIR LIBLUA51_LIBRARIES)

