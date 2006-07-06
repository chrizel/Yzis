
add_subdirectory( indent )

########### install files ###############

install_files( /share/yzis/scripts  FILES  2html.lua clevertab.lua switch.lua )



#original Makefile.am contents follow:

#SUBDIRS = indent
#scriptsdir = $(prefix)/share/yzis/scripts/
#scripts_DATA = 2html.lua clevertab.lua switch.lua
