# - Try to find the libmagic libraries
# Once done this will define
#
#  MAGIC_FOUND - system has libmagic
#  MAGIC_INCLUDE_DIR - the libmagic include directory
#  MAGIC_LIBRARIES - libmagic library

FIND_PATH(MAGIC_INCLUDE_DIR magic.h
   /usr/local/include
    ${GNUWIN32_INCLUDE_DIR}
)

FIND_LIBRARY(MAGIC_LIBRARIES NAMES magic
   PATHS
   /usr/lib
   /usr/local/lib
    ${GNUWIN32_LIBRARY_DIR}
)

if(MAGIC_INCLUDE_DIR AND MAGIC_LIBRARIES)
   set(MAGIC_FOUND TRUE)
else(MAGIC_INCLUDE_DIR AND MAGIC_LIBRARIES)
   set(MAGIC_FOUND "NO")
   if( Magic_FIND_REQUIRED)
      MESSAGE( FATAL_ERROR "libmagic could NOT BE FOUND!")
   endif( Magic_FIND_REQUIRED)
endif(MAGIC_INCLUDE_DIR AND MAGIC_LIBRARIES)


if(NOT MAGIC_FIND_QUIETLY)
	if(MAGIC_FOUND)
		message(STATUS "Found libmagic: ${MAGIC_LIBRARIES}")
	else(MAGIC_FOUND)
		message(STATUS "libmagic NOT FOUND !")
	endif(MAGIC_FOUND)
endif(NOT MAGIC_FIND_QUIETLY)

MARK_AS_ADVANCED(MAGIC_INCLUDE_DIR MAGIC_LIBRARIES)
