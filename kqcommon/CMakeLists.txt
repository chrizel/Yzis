kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = . 
#
#INCLUDES = $(YZIS_INCLUDES) $(LUA_INCLUDES)
#noinst_LTLIBRARIES = libkqcommon.la
#
#libkqcommon_la_SOURCES = keyConverter.cpp 
#libkqcommon_la_PCH = AUTO
#
#DISTCLEANFILES = translator.h
#
#re : clean all
#
#METASOURCES = AUTO
