
# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
#SUBDIRS = newui  pics .
SUBDIRS =  pics .

# set the include path for X, qt and KDE
#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/qyzis/newui/ -I$(top_srcdir)/qyzis/ -I$(top_srcdir)/libyzis/ -I$(top_builddir)/libyzis/ $(all_includes) $(LUA_INCLUDES)
INCLUDES = -I$(top_srcdir)  -I$(top_srcdir)/qyzis/ -I$(top_srcdir)/libyzis/ -I$(top_builddir)/libyzis/ $(all_includes) $(LUA_INCLUDES)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

bin_PROGRAMS = qyzis

# the application source, library search path, and link libraries
qyzis_SOURCES = main.cpp  viewwidget.cpp factory.cpp \
	editor.cpp commandwidget.cpp  \
	cursor.cpp qyzis.cpp 
qyzis_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#qyzis_LDADD =  $(LIB_KPARTS) $(top_builddir)/libyzis/libyzis.la   $(top_builddir)/qyzis/newui/libd.la
qyzis_LDADD =   $(LIB_QTGUI) $(top_builddir)/libyzis/libyzis.la # $(top_builddir)/qyzis/newui/libd.la

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Editors
shelldesktop_DATA = qyzis.desktop

# these are the headers for your project
noinst_HEADERS = commandwidget.h  cursor.h \
	editor.h  factory.h  qyzis.h  viewwidget.h 






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


install-exec-hook:
	-@for i in `ls $(top_builddir)/translations/*.gmo`; do \
		if test ! -f $(DESTDIR)/`$(KDECONFIG) --install locale --expandvars`/`basename $$i .gmo`/LC_MESSAGES/yzis.mo ; then \
			$(mkinstalldirs) $(DESTDIR)/`$(KDECONFIG) --install locale --expandvars`/`basename $$i .gmo`/LC_MESSAGES/ ; \
			$(LN_S) $(DESTDIR)$(prefix)/share/locale/`basename $$i .gmo`/LC_MESSAGES/yzis.mo $(DESTDIR)/`$(KDECONFIG) --install locale --expandvars`/`basename $$i .gmo`/LC_MESSAGES/ ; \
		fi ; \
	done

