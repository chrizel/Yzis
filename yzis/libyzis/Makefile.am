INCLUDES = -I$(TINYQ_INCLUDES)

#static lib
#lib_LIBRARIES = libstaticyzis.a
#libstaticyzis_a_SOURCES = yz_buffer.cpp yz_view.cpp yz_line.cpp yz_events.cpp yz_session.cpp
#libstaticyzis_a_LDFLAGS = -version-info 0:0:0 -no-undefined
#libstaticyzis_a_LIBADD = 	$(TINYQ_LIBS)/libtinyq.a

#lib_LIBRARIES = libyzis.a
#libyzis_a_SOURCES = yz_buffer.cpp.cc yz_view.cpp.cc yz_line.cpp.cc yz_events.cpp.cc yz_session.cpp.cc
#libyzis_a_LIBADD = $(TINYQ_LIBS)/libtinyq.a

## Rule to create static lib for nyzis (also convenience lib for the libtool lib) ##
lib_LIBRARIES = libyzis.a
libyzis_a_SOURCES = yz_buffer.cpp yz_view.cpp yz_line.cpp yz_events.cpp yz_session.cpp
#if we add this, we won't be able to use qt in kyzis i think
#libyzis_a_LIBADD = $(TINYQ_LIBS)/libtinyq.a

## Rule to create the libtool version from the static lib ##
lib_LTLIBRARIES = libyzis.la
libyzis_la_SOURCES = dummy.cc libyzis.a
libyzis_la_LDFLAGS = -version-info 0:0:0 -no-undefined 
libyzis_la_LIBADD = -L$(TINYQ_LIBS) -ltinyq 

re : clean all

dummy.cc:
	echo "" > dummy.cc
