bin_PROGRAMS = nyzis

INCLUDES = -I$(top_srcdir)/libyzis/ $(YZIS_INCLUDES)
nyzis_SOURCES = main.cpp nyz_view.cpp nyz_session.cpp
#nyzis_LDADD = $(top_builddir)/libyzis/libtinyqyzis/libtinyqyzis.la -lcurses
nyzis_LDADD = $(top_builddir)/$(NYZIS_DEP) -lcurses

re : clean all

