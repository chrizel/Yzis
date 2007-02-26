# - Try to find the libncurses(w) libraries
# Once done this will define
#
#  NCURSES_FOUND - system has libncurses(w)
#  NCURSES_INCLUDE_DIR - the libncurses include directory
#  NCURSES_LIBRARIES - libncurses(w) library

#to be completed I guess ...
FIND_PATH(NCURSES_INCLUDE_DIR ncursesw/cursesw.h
   /usr/include/
   /usr/local/include/
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

    if(NCURSES_LIBRARIES)
       set(NCURSES_FOUND TRUE)
    else(NCURSES_LIBRARIES)
       set(NCURSES_FOUND FALSE)
    endif(NCURSES_LIBRARIES)

else (NOT NCURSES_LIBRARIES)
   set(NCURSESW_FOUND TRUE)
   set(NCURSES_FOUND TRUE)
endif (NOT NCURSES_LIBRARIES)

#make sure that libncurses.so (not being called libncursesw) is wide-character-enabled
#we should find a way to add the link directory

if (NOT NCURSESW_FOUND AND NCURSES_FOUND)
   INCLUDE (CheckLibraryExists)
   CHECK_LIBRARY_EXISTS(ncurses mvwaddwstr "" HAVE_NCURSESW)
endif (NOT NCURSESW_FOUND AND NCURSES_FOUND)

if(NCURSES_FOUND)
   if(NOT NCURSES_FIND_QUIETLY)
      message(STATUS "Found libncurses(w): ${NCURSES_LIBRARIES}")
   endif(NOT NCURSES_FIND_QUIETLY)
endif(NCURSES_FOUND)

MARK_AS_ADVANCED(NCURSES_INCLUDE_DIR NCURSES_LIBRARIES)
