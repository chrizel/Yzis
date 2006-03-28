kde4_header()

include_directories( ${NCURSES_INCLUDE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/libyzis/ ${CMAKE_BINARY_DIR}/libyzis/ ${QT_INCLUDES} )


########### next target ###############

set(nyzis_SRCS viewwidget.cpp factory.cpp main.cpp )

kde4_automoc(${nyzis_SRCS})
kde4_add_executable(nyzis ${nyzis_SRCS})

target_link_libraries(nyzis ${NCURSES_LIBRARIES} ${X11_X11_LIB} ${QT_QTCORE_LIBRARY} yzis )

install_targets(/bin nyzis )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#bin_PROGRAMS = yzis
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/libyzis/ -I$(top_builddir)/libyzis/ $(YZIS_INCLUDES) $(LUA_INCLUDES) $(CURSES_INCLUDES)
#yzis_SOURCES = viewwidget.cpp factory.cpp main.cpp
#yzis_LDADD = $(top_builddir)/libyzis/libyzis.la $(YZISLIBS)
#
#re : clean all
#uncore :
#	gdb  `which yzis` core
#
#install-data-local :
#	@-rm -f $(DESTDIR)$(kde_bindir)/yz
#	@$(LN_S) yzis $(DESTDIR)$(kde_bindir)/yz
