kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(test_yzcolor_SRCS test_yzcolor.cpp )

kde4_automoc(${test_yzcolor_SRCS})

kde4_add_executable(test_yzcolor ${test_yzcolor_SRCS})

target_link_libraries(test_yzcolor  ${KDE4_KDECORE_LIBS} yzis )

install_targets(/bin test_yzcolor )


########### next target ###############

set(test_drawbuffer_SRCS test_drawbuffer.cpp )

kde4_automoc(${test_drawbuffer_SRCS})

kde4_add_executable(test_drawbuffer ${test_drawbuffer_SRCS})

target_link_libraries(test_drawbuffer  ${KDE4_KDECORE_LIBS} yzis )

install_targets(/bin test_drawbuffer )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#bin_PROGRAMS = test_yzcolor test_drawbuffer
#
#INCLUDES = -I$(top_srcdir) $(YZIS_INCLUDES)
#
#test_drawbuffer_SOURCES = test_drawbuffer.cpp
#test_drawbuffer_LDADD = $(top_builddir)/libyzis/libyzis.la $(YZISLIBS)
#test_yzcolor_SOURCES = test_yzcolor.cpp
#test_yzcolor_LDADD = $(top_builddir)/libyzis/libyzis.la $(YZISLIBS)
#
