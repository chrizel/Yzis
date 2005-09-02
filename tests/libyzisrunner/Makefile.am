bin_PROGRAMS = libyzisrunner

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libyzis/ -I$(top_builddir)/libyzis/ $(YZIS_INCLUDES) $(LUA_INCLUDES) $(CURSES_INCLUDES)
libyzisrunner_SOURCES = main.cpp
libyzisrunner_LDADD = $(top_builddir)/libyzis/libyzis.la $(YZISLIBS)


