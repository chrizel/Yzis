#
# $Id: makefile.pro,v 1.4 2004/01/10 16:04:52 philippe Exp $
#

TEMPLATE  = app
TARGET    = runTests
DESTDIR   = ./
CONFIG    += console warn_on debug qt
OBJECTS_DIR = obj

DEFINES += TEST

unix:LIBS += -Lcppunit/bin -lcppunit
unix:LIBS += -L../libyzis/libqtyzis/.libs -lqtyzis

INCLUDEPATH += cppunit/include cppunit/phil \
				..

HEADERS   =  testBuffer.h \
			 testCommands.h \
			 testUndo.h \
			 testSearch.h \
			 testDebugBackend.h \
			 TView.h \
			 TBuffer.h \
			 TSession.h
			 
SOURCES   = testBuffer.cpp \
			testCommands.cpp \
			testUndo.cpp \
			testSearch.cpp \
			testDebugBackend.cpp \
			TView.cpp \
			main.cpp
