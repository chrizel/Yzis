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

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
PKGCONFIG(lua50 _libLua50IncDir _libLua50LinkDir _libLua50LinkFlags _libLua50Cflags)

set(LIBLUA50_DEFINITIONS ${_libLua50Cflags})

# under windows, try to find the base gnuwin32 directory, do nothing under UNIX
FIND_PACKAGE(GNUWIN32)

FIND_PATH(LIBLUA50_INCLUDE_DIR lua.h
   ${_libLua50IncDir}/lua50
   /usr/include/lua50
   /usr/local/include/lua50
   ${GNUWIN32_DIR}/include
)

FIND_LIBRARY(LIBLUA50_LIBRARIES NAMES lua50
   PATHS
   ${_libLua50LinkDir}
   /usr/lib
   /usr/local/lib
   ${GNUWIN32_DIR}/lib
)


if (LIBLUA50_INCLUDE_DIR AND LIBLUA50_LIBRARIES)
   set(LIBLUA50_FOUND TRUE)
endif (LIBLUA50_INCLUDE_DIR AND LIBLUA50_LIBRARIES)


if (LIBLUA50_FOUND)
   if (NOT LibLua50_FIND_QUIETLY)
      message(STATUS "Found liblua50: ${LIBLUA50_LIBRARIES}")
   endif (NOT LibLua50_FIND_QUIETLY)
else (LIBLUA50_FOUND)
   if (LibLua50_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find liblua50")
   endif (LibLua50_FIND_REQUIRED)
endif (LIBLUA50_FOUND)

MARK_AS_ADVANCED(LIBLUA50_INCLUDE_DIR LIBLUA50_LIBRARIES)

