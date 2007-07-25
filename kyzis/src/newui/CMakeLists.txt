include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

SET(d_SRCS dmainwindow.cpp buttonbar.cpp button.cpp ddockwindow.cpp docksplitter.cpp dtabwidget.cpp)

ADD_LIBRARY(d STATIC ${d_SRCS} )

