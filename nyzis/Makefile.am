bin_PROGRAMS = yzis

INCLUDES = -I$(top_srcdir)/libyzis/ $(YZIS_INCLUDES)
yzis_SOURCES = main.cpp viewwidget.cpp factory.cpp document.cpp
yzis_LDADD = $(top_builddir)/$(NYZIS_DEP) -lcurses

re : clean all

