bin_PROGRAMS = nyzis

INCLUDES = -I$(top_srcdir)/libyzis/ -I$(TINYQ_INCLUDES)
nyzis_SOURCES = main.cpp nyz_view.cpp nyz_session.cpp
nyzis_LDADD = $(top_srcdir)/libyzis/libqtyzis.la -lcurses
#nyzis_LDFLAGS = $(KDE_RPATH) $(all_libraries)

re : clean all

