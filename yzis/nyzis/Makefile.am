bin_PROGRAMS = yzis

INCLUDES = -I$(top_srcdir)/libyzis/ $(YZIS_INCLUDES)
yzis_SOURCES = main.cpp nyz_view.cpp nyz_session.cpp
#yzis_LDADD = $(top_builddir)/libyzis/libtinyqyzis/libtinyqyzis.la -lcurses
yzis_LDADD = $(top_builddir)/$(NYZIS_DEP) -lcurses

re : clean all

