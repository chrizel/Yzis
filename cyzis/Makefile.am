bin_PROGRAMS = cyzis 

INCLUDES = -I$(top_srcdir)/libyzis/

cyzis_SOURCES = main.c dummy.cpp
cyzis_LDADD = $(top_srcdir)/libyzis/libyzis.la -lcurses

dummy.cpp :
	echo > dummy.cpp

