kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libyzis/ ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(libyzisrunner_SRCS main.cpp )

kde4_automoc(${libyzisrunner_SRCS})

kde4_add_executable(libyzisrunner ${libyzisrunner_SRCS})

target_link_libraries(libyzisrunner  ${KDE4_KDECORE_LIBS} yzis )

install_targets(/bin libyzisrunner )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#bin_PROGRAMS = libyzisrunner
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libyzis/ -I$(top_builddir)/libyzis/ $(YZIS_INCLUDES) $(LUA_INCLUDES) $(CURSES_INCLUDES)
#libyzisrunner_SOURCES = main.cpp
#libyzisrunner_LDADD = $(top_builddir)/libyzis/libyzis.la $(YZISLIBS)
#
#
