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

INCLUDEPATH += cppunit/include \
				..

HEADERS   =  testYZBuffer.h \
			 PhilAsserts.h \
			 PhilTestRunner.h
			 
SOURCES   = PhilAsserts.cpp \
			PhilTestRunner.cpp \
			testYZBuffer.cpp \
			main.cpp
