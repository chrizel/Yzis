bin_PROGRAMS = nyzis

INCLUDES = -I$(top_srcdir)/libyzis/
nyzis_SOURCES = main.cpp nyz_view.cpp nyz_session.cpp
nyzis_LDADD = $(top_srcdir)/libyzis/libyzis.la -lcurses

re : clean all

