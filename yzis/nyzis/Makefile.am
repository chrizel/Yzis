bin_PROGRAMS = nyzis

INCLUDES = -I$(top_srcdir)/libyzis/ -I$(TINYQ_INCLUDES)
nyzis_SOURCES = main.cpp nyz_view.cpp nyz_session.cpp $(TINYQ_LIBS)/libtinyq.a
nyzis_LDADD = $(top_srcdir)/libyzis/libyzis.la -lcurses

re : clean all

