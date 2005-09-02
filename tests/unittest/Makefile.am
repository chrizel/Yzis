bin_PROGRAMS = yzistest

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libyzis/ -I$(top_builddir)/libyzis/ $(YZIS_INCLUDES) $(LUA_INCLUDES)
yzistest_SOURCES = main.cpp TSession.cpp
yzistest_LDADD = $(top_builddir)/libyzis/libyzis.la $(YZISLIBS) -lQtTest_debug

