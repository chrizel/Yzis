INCLUDES=$(QT_INCLUDES) $(all_includes)

lib_LTLIBRARIES = libqtyzis.la

libqtyzis_la_SOURCES = dummy.cpp
libqtyzis_la_LIBADD = ../libyzis.la $(LIB_QT) $(all_libraries) $(LUA_LIBS) $(PS_LIBS) $(LIBMAGIC_LIBS) $(LIBGETTEXT_LIBS)

dummy.cpp:
	echo "#ifdef _AIX" > $@
	echo "namespace {" >> $@
	echo "void *not_empty_file;" >> $@
	echo "}" >> $@
	echo "#endif" >> $@

libqtyzis_la_LDFLAGS = -version-info 0:0:0 -no-undefined
