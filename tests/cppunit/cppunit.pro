
TEMPLATE  = lib
DESTDIR = bin
TARGET    = cppunit
CONFIG    += console warn_off debug staticlib
VERSION = 1.8.0
win32-msvc: TMAKE_CFLAGS += /GX /GR
win32-msvc: TMAKE_CXXFLAGS += /GX /GR

OBJECTS_DIR = obj

INCLUDEPATH = include


HEADERS   =  \
	include/cppunit/Asserter.h	\
	include/cppunit/CompilerOutputter.h	\
	include/cppunit/Exception.h	\
	include/cppunit/NotEqualException.h	\
	include/cppunit/Outputter.h	\
	include/cppunit/Portability.h	\
	include/cppunit/SourceLine.h	\
	include/cppunit/SynchronizedObject.h	\
	include/cppunit/Test.h	\
	include/cppunit/TestAssert.h	\
	include/cppunit/TestCase.h	\
	include/cppunit/TestCaller.h	\
	include/cppunit/TestFailure.h	\
	include/cppunit/TestFixture.h	\
	include/cppunit/TestResult.h	\
	include/cppunit/TestResultCollector.h	\
	include/cppunit/TestSucessListener.h	\
	include/cppunit/TestSuite.h	\
	include/cppunit/TextOutputter.h	\
	include/cppunit/TextTestProgressListener.h	\
	include/cppunit/TextTestResult.h	\
	include/cppunit/TextTestRunner.h	\
	include/cppunit/TestListener.h	\
	include/cppunit/XmlOutputter.h	\
	include/cppunit/extensions/TestFactory.h	\
	include/cppunit/extensions/AutoRegisterSuite.h	\
	include/cppunit/extensions/HelperMacros.h	\
	include/cppunit/extensions/Orthodox.h	\
	include/cppunit/extensions/RepeatedTest.h	\
	include/cppunit/extensions/TestDecorator.h	\
	include/cppunit/extensions/TestFactoryRegistry.h	\
	include/cppunit/extensions/TestSetUp.h	\
	include/cppunit/extensions/TestSuiteBuilder.h	\
	include/cppunit/extensions/TestSuiteFactory.h	\
	include/cppunit/extensions/TypeInfoHelper.h	\
	phil/PhilAsserts.h \
	phil/PhilTestRunner.h

SOURCES   =  \
	src/cppunit/Asserter.cpp			\
	src/cppunit/CompilerOutputter.cpp	\
	src/cppunit/Exception.cpp			\
	src/cppunit/NotEqualException.cpp	\
	src/cppunit/RepeatedTest.cpp		\
	src/cppunit/SourceLine.cpp			\
	src/cppunit/SynchronizedObject.cpp	\
	src/cppunit/TestAssert.cpp			\
	src/cppunit/TestCase.cpp			\
	src/cppunit/TestFactoryRegistry.cpp	\
	src/cppunit/TestFailure.cpp			\
	src/cppunit/TestResult.cpp			\
	src/cppunit/TestResultCollector.cpp	\
	src/cppunit/TestRunner.cpp			\
	src/cppunit/TestSetUp.cpp			\
	src/cppunit/TestSucessListener.cpp	\
	src/cppunit/TestSuite.cpp			\
	src/cppunit/TextOutputter.cpp		\
	src/cppunit/TextTestProgressListener.cpp	\
	src/cppunit/TextTestResult.cpp		\
	src/cppunit/TypeInfoHelper.cpp		\
	src/cppunit/XmlOutputter.cpp		\
	phil/PhilAsserts.cpp				\
	phil/PhilTestRunner.cpp

win32:SOURCES += DllMain.cpp

