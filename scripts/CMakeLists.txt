kde4_header()

add_subdirectory( indent )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install_files(  FILES  2html.lua clevertab.lua switch.lua )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = indent
#scriptsdir = $(prefix)/share/yzis/scripts/
#scripts_DATA = 2html.lua clevertab.lua switch.lua
