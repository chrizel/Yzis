kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#
#
#noinst_LTLIBRARIES = libd.la
#libd_la_LDFLAGS = $(all_libraries)
#
#noinst_HEADERS = dmainwindow.h ddockwindow.h buttonbar.h  button.h  comdefs.h docksplitter.h dtabwidget.h
#libd_la_SOURCES = dmainwindow.cpp buttonbar.cpp \
#	button.cpp ddockwindow.cpp docksplitter.cpp dtabwidget.cpp
#libd_la_LIBADD = $(LIB_KPARTS)
