kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libyzis/ ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(yzis_SRCS viewwidget.cpp factory.cpp main.cpp )

kde4_automoc(${yzis_SRCS})

kde4_add_executable(yzis ${yzis_SRCS})

target_link_libraries(yzis  ${KDE4_KDECORE_LIBS} yzis )

install_targets(/bin yzis )


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
