SUBDIRS = philcppunit .
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/tests/cpp/philcppunit/ $(YZIS_INCLUDES) $(LUA_INCLUDES)
KDE_CXXFLAGS = $(USE_EXCEPTIONS) -DTEST
noinst_PROGRAMS = tests

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

tests_SOURCES = main.cpp testBuffer.cpp testCommands.cpp testDebugBackend.cpp testSearch.cpp testUndo.cpp TView.cpp  
tests_LDADD = $(LIB_QT) $(all_libraries) $(LUA_LIBS) $(PS_LIBS) -lmagic $(top_builddir)/libyzis/libqtyzis/libqtyzis.la $(top_builddir)/libyzis/libyzis.la $(top_builddir)/tests/cpp/philcppunit/libphilcppunit.la

