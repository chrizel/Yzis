# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = pics

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/libyzis/ $(all_includes) 

# these are the headers for your project
noinst_HEADERS = kyzis.h editor.h commandwidget.h viewwidget.h document.h factory.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kyzis.pot

#KDE_ICON = kyzis

# this Makefile creates both a KPart application and a KPart
#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kyzis

# the application source, library search path, and link libraries
kyzis_SOURCES = main.cpp kyzis.cpp kyzis.skel settings.kcfgc
kyzis_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kyzis_LDADD   = -lkmdi $(LIB_KPARTS) $(top_builddir)/libyzis/libqtyzis/libqtyzis.la

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = kyzis.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/kyzis
shellrc_DATA = kyzis_shell.rc

kde_kcfg_DATA = kyzis.kcfg

#########################################################################
# KPART SECTION
#########################################################################
kde_module_LTLIBRARIES = libkyzispart.la

# the Part's source, library search path, and link libraries
libkyzispart_la_SOURCES = document.cpp viewwidget.cpp factory.cpp editor.cpp commandwidget.cpp 
libkyzispart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkyzispart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) -lktexteditor $(top_builddir)/libyzis/libqtyzis/libqtyzis.la

# this is where the desktop file will go 
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = kyzispart.desktop

# this is where the part's XML-GUI resource file goes
partrcdir   = $(kde_datadir)/kyzispart
partrc_DATA = kyzispart.rc


#########################################################################
# MISC SECTION
#########################################################################
re : clean all

