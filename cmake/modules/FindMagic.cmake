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
endif(MAGIC_INCLUDE_DIR AND MAGIC_LIBRARIES)


if(MAGIC_FOUND)
   if(NOT MAGIC_FIND_QUIETLY)
      message(STATUS "Found libmagic: ${MAGIC_LIBRARIES}")
   endif(NOT MAGIC_FIND_QUIETLY)
endif(MAGIC_FOUND)

MARK_AS_ADVANCED(MAGIC_INCLUDE_DIR MAGIC_LIBRARIES)
