INCLUDES=$(QT_INCLUDES)

lib_LTLIBRARIES = libqt4yzis.la

libqt4yzis_la_SOURCES = dummy.cpp
libqt4yzis_la_LDFLAGS = -version-info 0:0:0 -no-undefined

dummy.cpp:
	echo "#ifdef _AIX" > $@
	echo "namespace {" >> $@
	echo "void *not_empty_file;" >> $@
	echo "}" >> $@
	echo "#endif" >> $@

libqt4yzis_la_LIBADD = ../libyzis.la $(QT_LIBS) -lQtCore_debug -lQtXml_debug -lQtGui_debug $(all_libraries) $(LUA_LIBS) $(PS_LIBS) -lmagic
