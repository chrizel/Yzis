INCLUDES=$(QT_INCLUDES)

lib_LTLIBRARIES = libqtyzis.la

libqtyzis_la_SOURCES = dummy.cpp
libqtyzis_la_LDFLAGS = -version-info 0:0:0 -no-undefined
libqtyzis_la_LIBADD = ../libyzis.la $(LIB_QT) $(all_libraries) $(LUA_LIBS) -lps -lmagic

dummy.cpp:
	echo "" > dummy.cpp

