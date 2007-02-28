# - Try to find the LibLua50 
# Once done this will define
#
#  LIBLUA50_FOUND - system has the LibLua50
#  LIBLUA50_INCLUDE_DIR - the LibLua50 include directory
#  LIBLUA50_LIBRARIES - The libraries needed to use LibLua50
# under Windows this also checks in the GNUWIN32 directory, so make
# sure that the GNUWIN32 directory gets found if you use the GNUWIN32 version of PCRE
# under UNIX pkgconfig among others pkg-config is used to find the directories


INCLUDE(UsePkgConfig)

message(STATUS "Looking for liblua (version 5.0)")

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
#debian
PKGCONFIG(lua50 _libLua50IncDir _libLua50LinkDir _libLua50LinkFlags _libLua50Cflags)
#gentoo
PKGCONFIG(lua _libLuaIncDir _libLuaLinkDir _libLuaLinkFlags _libLuaCflags)
#any other distrib to blame for using common naming ?
#

set(LIBLUA50_DEFINITIONS ${_libLua50Cflags} ${_libLuaCflags} )

# under windows, try to find the base gnuwin32 directory, do nothing under UNIX
FIND_PACKAGE(GNUWIN32)

FIND_PATH(LIBLUA50_INCLUDE_DIR lua.h
   ${WITH_LUA_HOME}/include
   ${_libLua50IncDir}/lua50
   ${_libLuaIncDir}
   /usr/include/lua50
   /usr/local/include/lua50
   ${GNUWIN32_DIR}/include
)

FIND_LIBRARY(LIBLUA50_LIBRARIES NAMES lua50 lua
   PATHS
   ${WITH_LUA_HOME}/lib
   ${_libLua50LinkDir}
   ${_libLuaLinkDir}
   /usr/lib
   /usr/local/lib
   ${GNUWIN32_DIR}/lib
)


# We have found something that looks like lua 5
# disambiguate between lua 5.0 and lua 5.1
if (LIBLUA50_INCLUDE_DIR AND LIBLUA50_LIBRARIES)
    # LUA_VERSION_NUM is only present in lua.h of version 5.1
    SET( CMAKE_REQUIRED_INCLUDES ${LIBLUA50_INCLUDE_DIR} )
    CHECK_INCLUDE_FILES( luaconf.h HAVE_LUACONF_FOR_51_H )
    if ( HAVE_LUACONF_FOR_51_H )
       set(LIBLUA51_FOUND TRUE)
    else ( HAVE_LUACONF_FOR_51_H )
       set(LIBLUA50_FOUND TRUE)
    endif ( HAVE_LUACONF_FOR_51_H )
endif (LIBLUA50_INCLUDE_DIR AND LIBLUA50_LIBRARIES)

# uh uh, the wrong version is installed
if (LIBLUA51_FOUND)
   message(STATUS "Looking for liblua (version 5.0) but liblua (version 5.1) was found : ${LIBLUA50_LIBRARIES}")
endif (LIBLUA51_FOUND)

if (LIBLUA50_FOUND)
   if (NOT LibLua50_FIND_QUIETLY)
      message(STATUS "Looking for liblua(50) - found: ${LIBLUA50_LIBRARIES}")
   endif (NOT LibLua50_FIND_QUIETLY)
else (LIBLUA50_FOUND)
    if (LibLua50_FIND_REQUIRED)
       message(FATAL_ERROR "Looking for liblua (version 5.0) - not found")
    endif (LibLua50_FIND_REQUIRED)
endif (LIBLUA50_FOUND)

MARK_AS_ADVANCED(LIBLUA50_INCLUDE_DIR LIBLUA50_LIBRARIES)


