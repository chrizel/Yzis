# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libyzis/ $(all_includes) $(LUA_INCLUDES)

# these are the headers for your project
noinst_HEADERS = commandwidget.h  configdialog.h  cursor.h  document.h  editor.h  factory.h  konsole.h  kyziscodecompletion.h  kyzis.h  viewwidget.h hlconfig.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

########## intern lib ##########
noinst_LTLIBRARIES = libinternkyzis.la
libinternkyzis_la_SOURCES = document.cpp viewwidget.cpp factory.cpp settings.kcfgc editor.cpp commandwidget.cpp configdialog.cpp kyziscodecompletion.cpp cursor.cpp kyzis.cpp kyzis.skel konsole.cpp hlconfig.cpp

kde_kcfg_DATA = kyzis.kcfg
 
re : clean all

