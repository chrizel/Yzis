# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = src pics .

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kyzis/src/ -I$(top_srcdir)/libyzis/ $(all_includes) $(LUA_INCLUDES)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

bin_PROGRAMS = kyzis

# the application source, library search path, and link libraries
kyzis_SOURCES = main.cpp
kyzis_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kyzis_LDADD = -lktexteditor -lkmdi $(LIB_KPARTS) $(top_builddir)/libyzis/libqtyzis/libqtyzis.la $(top_builddir)/kyzis/src/libinternkyzis.la

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Editors
shelldesktop_DATA = kyzis.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/kyzis
shellrc_DATA = kyzis_shell.rc


kde_module_LTLIBRARIES = libkyzispart.la

# the Part's source, library search path, and link libraries
libkyzispart_la_SOURCES = dummy.cpp
libkyzispart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkyzispart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) -lkmdi -lktexteditor $(top_builddir)/libyzis/libqtyzis/libqtyzis.la $(top_builddir)/kyzis/src/libinternkyzis.la

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

dummy.cpp: 
	echo "#ifdef _AIX" > $@
	echo "namespace {" >> $@
	echo "void *not_empty_file;" >> $@
	echo "}" >> $@
	echo "#endif" >> $@


