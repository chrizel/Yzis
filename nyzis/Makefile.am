bin_PROGRAMS = yzis

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libyzis/ $(YZIS_INCLUDES) $(LUA_INCLUDES)
yzis_SOURCES = main.cpp viewwidget.cpp factory.cpp document.cpp eventloop.cpp
yzis_LDADD = $(top_builddir)/$(NYZIS_DEP) $(YZISLIBS)

re : clean all
uncore :
	gdb  `which yzis` core

install-data-local :
	@-rm -f $(DESTDIR)$(kde_bindir)/yz
	@$(LN_S) yzis $(DESTDIR)$(kde_bindir)/yz
