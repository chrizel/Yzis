
#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>

#include <iostream>

#include <cppunit/TestResult.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/SourceLine.h>
#include <cppunit/Exception.h>

#include "PhilTestRunner.h"

// --------- This belongs to CppUnit

namespace CppUnit {

bool
ExtendedTestRunner::runTest( Test *test,
                         bool doPrintProgress )
{
  if ( doPrintProgress )
    m_eventManager->addListener( m_progressListener );

  test->run( m_eventManager );

  if ( doPrintProgress )
    m_eventManager->removeListener( m_progressListener );
  return m_result->wasSuccessful();
}

};
// -------------------------------------------------


// My simple runner
PhilTestRunner::PhilTestRunner()
{
	setTestProgressListener( new PhilProgressListener() );
	setOutputter( new PhilOutputter( m_result ) );
}

// -------------------------------------------------

// My progress listener

void printTestName(CppUnit::Test *test) {
	static std::string _currentSuite = "";

	std::string s = test->getName();
	std::string tc_s = "TestCaller ";

	if (s.find( tc_s ) != std::string::npos ) {
		s.erase( s.find( tc_s), s.find( tc_s ) + tc_s.size() );
	}

	if (s.find(".") != std::string::npos ) {
		std::string suiteName;
		std::string testName;

		suiteName = s.substr(0, s.find(".") );
		testName = s.substr( s.find(".") + 1 );

		if (suiteName != _currentSuite) {
			std::cerr << std::endl;
			std::cerr << ">>>>>>>>      suite " << suiteName << "     <<<<<<<<<" << std::endl;
			_currentSuite = suiteName;
		}
		std::cerr << ">>> " << testName << std::endl;
	} else {
		std::cerr << s  << std::endl;
	}
}
void PhilProgressListener::startTest (CppUnit::Test *test) {
	printTestName( test );
}


void PhilProgressListener::endTest (CppUnit::Test * /*test*/) {
//	std::cerr << "." << std::endl;
}


void printFailure (const CppUnit::TestFailure &failure) {
	CppUnit::SourceLine sl = failure.thrownException()->sourceLine();

	char s[1024];
	char errorFormat[1024];

#ifdef WIN32
	sprintf( errorFormat, "%s", "%s(%d) : %s: " );
#else
	sprintf( errorFormat, "[;1;35m%s[;0;37;49m", "%s:%d: %s: " );
#endif

	strcpy( s, "" );
	if( sl.isValid() ) {
		sprintf( s, errorFormat, sl.fileName().c_str(), sl.lineNumber(),
			failure.isError() ? "Error" : "Failure" );
	}
	strcat( s, failure.thrownException()->what());
	std::cerr << s << std::endl;

#ifdef WIN32
	OutputDebugStringA( s );
#endif


}

void PhilProgressListener::addFailure (const CppUnit::TestFailure &failure) {
	printFailure( failure );
}



// -------------------------------------------------

void PhilOutputter::write() {

	int all_nb = m_resultCollector->runTests();
	int success_nb = all_nb - m_resultCollector->testFailuresTotal();
	int success_percent = 0;
	if (all_nb > 0) {
		success_percent = (int) (100.0 * success_nb / all_nb);
	}

	std::cerr << std::endl << "Success : " << success_percent << "% - " << success_nb
		   << " of " << all_nb << std::endl;

	if (m_resultCollector->wasSuccessful() == false) {
		std::cerr << "Failures : " << m_resultCollector->testFailures() << std::endl;
		std::cerr << "Errors   : " << m_resultCollector->testErrors() << std::endl;

		CppUnit::TestResultCollector::TestFailures::const_iterator it;
		it =( m_resultCollector->failures().begin() );
		for( ; it != m_resultCollector->failures().end(); ++it ) {
			std::cerr << ">>>" << (*it)->failedTestName() << std::endl;
			printFailure( **it );
		}
		// display all failures, with the name of th
	}
	std::cerr << std::endl;

/*
#ifndef WIN32
	exit(0);
#endif
	*/
}

