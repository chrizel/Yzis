bin_PROGRAMS = yzis

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libyzis/ -I$(top_builddir)/libyzis/ $(YZIS_INCLUDES) $(LUA_INCLUDES) $(CURSES_INCLUDES)
yzis_SOURCES = viewwidget.cpp factory.cpp main.cpp
yzis_LDADD = $(top_builddir)/libyzis/libqtyzis/libqtyzis.la $(YZISLIBS)

re : clean all
uncore :
	gdb  `which yzis` core

install-data-local :
	@-rm -f $(DESTDIR)$(kde_bindir)/yz
	@$(LN_S) yzis $(DESTDIR)$(kde_bindir)/yz
