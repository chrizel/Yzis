kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libyzis/ ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(gyzis_SRCS 
   main.cpp 
   gview.cpp 
   document.cpp 
   gyzis.cpp )

kde4_automoc(${gyzis_SRCS})

kde4_add_executable(gyzis ${gyzis_SRCS})

target_link_libraries(gyzis  ${KDE4_KDECORE_LIBS} qtgtk yzis )

install_targets(/bin gyzis )


########### install files ###############

install_files( ${APPLNK_INSTALL_DIR}/Editeurs FILES  gyzis.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/libyzis/ $(all_includes) $(LUA_INCLUDES) -I/usr/include/bakery-2.4/ `pkg-config bakery-2.4 --cflags`
#
#METASOURCES = AUTO
#
#applnkediteurs_DATA = gyzis.desktop
#applnkediteursdir = $(kde_appsdir)/Editeurs
#
#bin_PROGRAMS = gyzis
#gyzis_SOURCES = main.cpp gview.cpp document.cpp gyzis.cpp
#gyzis_LDFLAGS = $(all_libraries) `pkg-config bakery-2.4 --libs` -lqtgtk
#gyzis_LDADD = $(top_builddir)/libyzis/libyzis.la
#
#
#
