kde4_header()

add_subdirectory( newui )

include_directories( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/kyzis/src/newui/ ${CMAKE_SOURCE_DIR}/libyzis/ ${CMAKE_BINARY_DIR}/libyzis/ ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############
add_definitions("-DQT3_SUPPORT")
SET(internkyzis_SRCS ktexteditoriface.cpp viewwidget.cpp factory.cpp editor.cpp commandwidget.cpp kyziscodecompletion.cpp cursor.cpp kyzis.cpp konsole.cpp kyzisbuffer.cpp)

KDE4_AUTOMOC (${internkyzis_SRCS})
KDE4_ADD_DCOP_SKELS (internkyzis_SRCS kyzis.h)
KDE4_ADD_KCFG_FILES (internkyzis_SRCS settings.kcfgc)

ADD_LIBRARY(internkyzis STATIC ${internkyzis_SRCS} )

########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES  kyzis.kcfg )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = newui
#
## set the include path for X, qt and KDE
#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kyzis/src/newui/ -I$(top_srcdir)/libyzis/ -I$(top_builddir)/libyzis/ $(all_includes) $(LUA_INCLUDES)
#
## these are the headers for your project
#noinst_HEADERS = commandwidget.h  cursor.h  ktexteditoriface.h \
#	editor.h  factory.h  konsole.h  kyziscodecompletion.h  kyzis.h  viewwidget.h \
#	kyzisbuffer.h
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
########### intern lib ##########
#noinst_LTLIBRARIES = libinternkyzis.la
#libinternkyzis_la_SOURCES = ktexteditoriface.cpp viewwidget.cpp factory.cpp \
#	settings.kcfgc editor.cpp commandwidget.cpp kyziscodecompletion.cpp \
#	cursor.cpp kyzis.cpp kyzis.skel konsole.cpp kyzisbuffer.cpp 
#libinternkyzis_la_LIBADD = $(top_builddir)/kyzis/src/newui/libd.la -lktexteditor
#
#kde_kcfg_DATA = kyzis.kcfg
# 
#re : clean all
#
