INCLUDES = -I$(top_srcdir)/libyzis/ $(all_includes) $(LUA_INCLUDES) -I/usr/include/bakery-2.4/ `pkg-config bakery-2.4 --cflags`

METASOURCES = AUTO

applnkediteurs_DATA = gyzis.desktop
applnkediteursdir = $(kde_appsdir)/Editeurs

bin_PROGRAMS = gyzis
gyzis_SOURCES = main.cpp gview.cpp document.cpp gyzis.cpp
gyzis_LDFLAGS = $(all_libraries) `pkg-config bakery-2.4 --libs` -lqtgtk
gyzis_LDADD = $(top_builddir)/libyzis/libyzis.la



