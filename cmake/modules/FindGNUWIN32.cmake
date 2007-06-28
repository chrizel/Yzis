if (WIN32)

    FILE(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _progFiles)
    FILE(TO_CMAKE_PATH "$ENV{GNUWIN32_DIR}" _gnuwin32Dir)
    # message( _gnuwin32Dir=${_gnuwin32Dir} )

    IF (_gnuwin32Dir AND EXISTS ${_gnuwin32Dir})
        # there is an environement variable for it!
        set( GNUWIN32_DIR ${_gnuwin32Dir} )
    else ()
        # we are on our own to find it
        FIND_FILE(GNUWIN32_DIR gnuwin32
           ${_progFiles}
           "C:/"
        )
    endif ()

    if (GNUWIN32_DIR)
       set(GNUWIN32_INCLUDE_DIR ${GNUWIN32_DIR}/include)
       set(GNUWIN32_LIBRARY_DIR ${GNUWIN32_DIR}/lib)
       set(GNUWIN32_BINARY_DIR  ${GNUWIN32_DIR}/bin)
       set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${GNUWIN32_INCLUDE_DIR})
       set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${GNUWIN32_LIBRARY_DIR})
       set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${GNUWIN32_INCLUDE_DIR})
       set(GNUWIN32_FOUND TRUE)
    else (GNUWIN32_DIR)
       set(GNUWIN32_FOUND)
    endif (GNUWIN32_DIR)

    if (GNUWIN32_FOUND)
      if (NOT GNUWIN32_FIND_QUIETLY)
        message(STATUS "Found GNUWIN32: ${GNUWIN32_DIR}")
      endif (NOT GNUWIN32_FIND_QUIETLY)
    else (GNUWIN32_FOUND)
      if (GNUWIN32_FIND_REQUIRED)
        message(SEND_ERROR "Could NOT find GNUWIN32")
      endif (GNUWIN32_FIND_REQUIRED)
    endif (GNUWIN32_FOUND)

endif (WIN32)

