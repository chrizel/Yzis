kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libyzis/ ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(yzistest_SRCS main.cpp TSession.cpp )

kde4_automoc(${yzistest_SRCS})

kde4_add_executable(yzistest ${yzistest_SRCS})

target_link_libraries(yzistest  ${KDE4_KDECORE_LIBS} yzis QtTest_debug )

install_targets(/bin yzistest )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#bin_PROGRAMS = yzistest
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libyzis/ -I$(top_builddir)/libyzis/ $(YZIS_INCLUDES) $(LUA_INCLUDES)
#yzistest_SOURCES = main.cpp TSession.cpp
#yzistest_LDADD = $(top_builddir)/libyzis/libyzis.la $(YZISLIBS) -lQtTest_debug
#
