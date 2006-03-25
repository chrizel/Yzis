kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install_files(  FILES  cpp.lua python.lua )

kde4_footer()



#original Makefile.am contents follow:

#scriptsdir = $(prefix)/share/yzis/scripts/indent/
#scripts_DATA = cpp.lua python.lua
