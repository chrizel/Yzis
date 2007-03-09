# - Try to find the libncurses(w) libraries
# Once done this will define
#
#  NCURSES_FOUND - system has libncurses(w)
#  NCURSES_INCLUDE_DIR - the libncurses include directory
#  NCURSES_LIBRARIES - libncurses(w) library

message(STATUS "Looking for libncurses(w)")

#to be completed I guess ...
FIND_PATH(NCURSES_INCLUDE_DIR cursesw.h ncurses.h
   /usr/include/ncursesw/
   /usr/include/
   /usr/local/include/
   /usr/local/include/ncursesw/
    NO_DEFAULT_PATH
)

FIND_LIBRARY(NCURSES_LIBRARIES NAMES ncursesw
   PATHS
   /usr/lib
   /usr/local/lib
)

if (NOT NCURSES_LIBRARIES) 
   set(NCURSESW_FOUND FALSE)
   # we don't have ncursesw but we can still have ncurses compiled
   # with wide-charcter support.

   FIND_LIBRARY(NCURSES_LIBRARIES NAMES ncurses
	PATHS
	/usr/lib
	/usr/local/lib
   )
else (NOT NCURSES_LIBRARIES)
   set(NCURSESW_FOUND TRUE)
endif (NOT NCURSES_LIBRARIES)

if (NCURSES_LIBRARIES AND NCURSES_INCLUDE_DIR)
  set(NCURSES_FOUND TRUE)
else (NCURSES_LIBRARIES AND NCURSES_INCLUDE_DIR)
  set(NCURSES_FOUND FALSE)
  set(NCURSESW_FOUND FALSE)
endif (NCURSES_LIBRARIES AND NCURSES_INCLUDE_DIR)

# make sure that libncurses.so (not being called libncursesw) is
# wide-character-enabled we should find a way to add the link
# directory.
if (NCURSES_FOUND AND NOT NCURSESW_FOUND)
   INCLUDE (CheckLibraryExists)
   CHECK_LIBRARY_EXISTS(ncurses mvwaddwstr "" HAVE_NCURSESW)
endif (NCURSES_FOUND AND NOT NCURSESW_FOUND)

if(NCURSES_FOUND)
   if(NOT NCURSES_FIND_QUIETLY)
      message(STATUS "Looking for libncurses(w) - found")
   endif(NOT NCURSES_FIND_QUIETLY)
else(NCURSES_FOUND)
   if(NOT NCURSES_FIND_QUIETLY)
      message(STATUS "Looking for libncurses(w) - not found")
   endif (NOT NCURSES_FIND_QUIETLY)
endif (NCURSES_FOUND)

MARK_AS_ADVANCED(NCURSES_INCLUDE_DIR NCURSES_LIBRARIES)
