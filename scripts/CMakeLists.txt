
add_subdirectory( indent )

########### install files ###############

install( FILES 2html.lua clevertab.lua switch.lua DESTINATION share/yzis/scripts )



#original Makefile.am contents follow:

#SUBDIRS = indent
#scriptsdir = $(prefix)/share/yzis/scripts/
#scripts_DATA = 2html.lua clevertab.lua switch.lua
