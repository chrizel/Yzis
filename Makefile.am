INCLUDES = -I$(TINYQ_INCLUDES)

#static lib
#lib_LIBRARIES = libstaticyzis.a
#libstaticyzis_a_SOURCES = yz_buffer.cpp yz_view.cpp yz_line.cpp yz_events.cpp yz_session.cpp
#libstaticyzis_a_LDFLAGS = -version-info 0:0:0 -no-undefined
#libstaticyzis_a_LIBADD = 	$(TINYQ_LIBS)/libtinyq.a

#lib_LIBRARIES = libyzis.a
#libyzis_a_SOURCES = yz_buffer.cpp.cc yz_view.cpp.cc yz_line.cpp.cc yz_events.cpp.cc yz_session.cpp.cc
#libyzis_a_LIBADD = $(TINYQ_LIBS)/libtinyq.a

noinst_LTLIBRARIES = libyzis.la
lib_LTLIBRARIES = libqtyzis.la libtinyqyzis.la

libyzis_la_SOURCES = yz_buffer.cpp yz_view.cpp yz_line.cpp yz_events.cpp yz_session.cpp 

libqtyzis_la_SOURCES = dummy.cpp
libqtyzis_la_LDFLAGS = -version-info 0:0:0 -no-undefined 
libqtyzis_la_LIBADD = libyzis.la $(LIB_QT) $(all_libraries)

libtinyqyzis_la_SOURCES = dummy.cpp
libtinyqyzis_la_LDFLAGS = -version-info 0:0:0 -no-undefined 
libtinyqyzis_la_LIBADD = libyzis.la -L$(TINYQ_LIBS)/ -ltinyq 

dummy.cpp:
	echo "" > dummy.cpp
