INCLUDES=$(QT_INCLUDES)

lib_LTLIBRARIES = libqt4yzis.la

libqt4yzis_la_SOURCES = dummy.cpp
libqt4yzis_la_LDFLAGS = -version-info 0:0:0 -no-undefined
libqt4yzis_la_LIBADD = ../libyzis.la -lQtCore -lQt3Compat $(all_libraries) $(LUA_LIBS) $(PS_LIBS)

dummy.cpp:
	echo "" > dummy.cpp

