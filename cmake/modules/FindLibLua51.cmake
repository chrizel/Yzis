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

message(STATUS "Looking for liblua (version 5.1)")

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
#debian
PKGCONFIG(lua51 _libLua51IncDir _libLua51LinkDir _libLua51LinkFlags _libLua51Cflags)
#gentoo
PKGCONFIG(lua _libLuaIncDir _libLuaLinkDir _libLuaLinkFlags _libLuaCflags)
#any other distrib to blame for using common naming ?
#

set(LIBLUA51_DEFINITIONS ${_libLua51Cflags} ${_libLuaCflags} )

# under windows, try to find the base gnuwin32 directory, do nothing under UNIX
FIND_PACKAGE(GNUWIN32)

FIND_PATH(LIBLUA51_INCLUDE_DIR lua.h
   ${WITH_LUA_HOME}/include
   ${_libLua51IncDir}/lua51
   ${_libLua51IncDir}/lua5.1
   ${_libLuaIncDir}
   /usr/include/lua51
   /usr/include/lua5.1
   /usr/local/include/lua51
   /usr/local/include/lua5.1
   ${GNUWIN32_DIR}/include
   /usr/local/Cellar/lua/5.1.4/include
)

FIND_LIBRARY(LIBLUA51_LIBRARIES NAMES lua5.1 lua51 lua
   PATHS
   ${WITH_LUA_HOME}/lib
   ${_libLua51LinkDir}
   ${_libLuaLinkDir}
   /usr/lib
   /usr/local/lib
   /usr/local/lib/lua51
   ${GNUWIN32_DIR}/lib
   /usr/local/Cellar/lua/5.1.4/lib
)


# We have found something that looks like lua 5
# disambiguate between lua 5.1 and lua 5.0
if (LIBLUA51_INCLUDE_DIR AND LIBLUA51_LIBRARIES)
    # LUA_VERSION_NUM is only present in lua.h of version 5.1
    SET( CMAKE_REQUIRED_INCLUDES ${LIBLUA51_INCLUDE_DIR} )
    CHECK_INCLUDE_FILES( luaconf.h HAVE_LUACONF_H_FOR_LUA_51 )
    if ( HAVE_LUACONF_H_FOR_LUA_51 )
       set(LIBLUA51_FOUND TRUE)
    else ( HAVE_LUACONF_H_FOR_LUA_51 )
       set(LIBLUA50_FOUND TRUE)
    endif ( HAVE_LUACONF_H_FOR_LUA_51 )
endif (LIBLUA51_INCLUDE_DIR AND LIBLUA51_LIBRARIES)

# uh uh, the wrong version is installed
if (LIBLUA50_FOUND)
   message(STATUS "Looking for liblua (version 5.1) but liblua (version 5.0) was found : ${LIBLUA51_LIBRARIES}")
endif (LIBLUA50_FOUND)

if (LIBLUA51_FOUND)
   if (NOT LibLua51_FIND_QUIETLY)
      message(STATUS "Looking for liblua(51) - found: ${LIBLUA51_LIBRARIES}")
   endif (NOT LibLua51_FIND_QUIETLY)
else (LIBLUA51_FOUND)
    if (LibLua51_FIND_REQUIRED)
       message(FATAL_ERROR "Looking for liblua (version 5.1) - not found")
    endif (LibLua51_FIND_REQUIRED)
endif (LIBLUA51_FOUND)

MARK_AS_ADVANCED(LIBLUA51_INCLUDE_DIR LIBLUA51_LIBRARIES)


