bin_PROGRAMS = nyzis

INCLUDES = -I$(top_srcdir)/libyzis/ $(TINYQ_INCLUDES)
nyzis_SOURCES = main.cpp nyz_view.cpp nyz_session.cpp
nyzis_LDADD = $(top_builddir)/libyzis/libtinyqyzis/libtinyqyzis.la -lcurses

re : clean all

