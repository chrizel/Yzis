bin_PROGRAMS = yzis

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libyzis/ $(YZIS_INCLUDES) $(LUA_INCLUDES) $(CURSES_INCLUDES)
yzis_SOURCES = viewwidget.cpp factory.cpp document.cpp eventloop.cpp main.cpp
if use_libqt4
yzis_LDADD = $(top_builddir)/$(NYZIS_DEP) $(YZISLIBS) -lQtGui_debug -lQtCore_debug -lQtXml_debug
else
yzis_LDADD = $(top_builddir)/$(NYZIS_DEP) $(YZISLIBS)
endif

re : clean all
uncore :
	gdb  `which yzis` core

install-data-local :
	@-rm -f $(DESTDIR)$(kde_bindir)/yz
	@$(LN_S) yzis $(DESTDIR)$(kde_bindir)/yz
