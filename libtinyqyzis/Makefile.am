INCLUDES=$(TINYQ_INCLUDES)

lib_LTLIBRARIES = libtinyqyzis.la

libtinyqyzis_la_SOURCES = dummy.cpp
libtinyqyzis_la_LDFLAGS = -version-info 0:0:0 -no-undefined
libtinyqyzis_la_LIBADD = ../libyzis.la $(TINYQ_LIBS) $(LUA_LIBS)

dummy.cpp:
	echo "" > dummy.cpp

