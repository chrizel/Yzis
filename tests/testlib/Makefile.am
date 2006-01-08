
bin_PROGRAMS = test_yzcolor test_drawbuffer

INCLUDES = -I$(top_srcdir) $(YZIS_INCLUDES)

test_drawbuffer_SOURCES = test_drawbuffer.cpp
test_drawbuffer_LDADD = $(top_builddir)/libyzis/libyzis.la $(YZISLIBS)
test_yzcolor_SOURCES = test_yzcolor.cpp
test_yzcolor_LDADD = $(top_builddir)/libyzis/libyzis.la $(YZISLIBS)

