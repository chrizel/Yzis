# - Try to find the libmagic libraries
# Once done this will define
#
#  MAGIC_FOUND - system has libmagic
#  MAGIC_INCLUDE_DIR - the libmagic include directory
#  MAGIC_LIBRARIES - libmagic library

# under windows, try to find the base gnuwin32 directory, do nothing under UNIX
FIND_PACKAGE(GNUWIN32)

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

if (MAGIC_FOUND)
   if (NOT Gettext_FIND_QUIETLY)
      message(STATUS "Found libmagic: ${MAGIC_LIBRARIES}")
   endif (NOT Gettext_FIND_QUIETLY)
else (MAGIC_FOUND)
   if (Gettext_FIND_REQUIRED)
      message(FATAL_ERROR "libmagic NOT FOUND !")
   else (Gettext_FIND_REQUIRED)
      if (NOT Gettext_FIND_QUIETLY)
         message(STATUS "libmagic NOT FOUND !")
      endif (NOT Gettext_FIND_QUIETLY)
   endif (Gettext_FIND_REQUIRED)
endif (MAGIC_FOUND)


MARK_AS_ADVANCED(MAGIC_INCLUDE_DIR MAGIC_LIBRARIES)
