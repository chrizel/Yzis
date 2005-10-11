SUBDIRS = newui

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kyzis/src/newui/ -I$(top_srcdir)/libyzis/ -I$(top_builddir)/libyzis/ $(all_includes) $(LUA_INCLUDES)

# these are the headers for your project
noinst_HEADERS = commandwidget.h  cursor.h  ktexteditoriface.h \
	editor.h  factory.h  konsole.h  kyziscodecompletion.h  kyzis.h  viewwidget.h \
	kyzisbuffer.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

########## intern lib ##########
noinst_LTLIBRARIES = libinternkyzis.la
libinternkyzis_la_SOURCES = ktexteditoriface.cpp viewwidget.cpp factory.cpp \
	settings.kcfgc editor.cpp commandwidget.cpp kyziscodecompletion.cpp \
	cursor.cpp kyzis.cpp kyzis.skel konsole.cpp kyzisbuffer.cpp 
libinternkyzis_la_LIBADD = $(top_builddir)/kyzis/src/newui/libd.la -lktexteditor

kde_kcfg_DATA = kyzis.kcfg
 
re : clean all

