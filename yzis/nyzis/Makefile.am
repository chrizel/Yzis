bin_PROGRAMS = nyzis

INCLUDES = -I$(top_srcdir)/libyzis/ -I$(TINYQ_INCLUDES)
nyzis_SOURCES = main.cpp nyz_view.cpp nyz_session.cpp
nyzis_LDADD = $(top_srcdir)/libyzis/libyzis.a $(TINYQ_LIBS)/libtinyq.a -lcurses
nyzis_LDFLAGS = -static 

re : clean all

