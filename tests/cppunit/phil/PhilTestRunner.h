
#ifndef PHIL_TEST_OUTPUTTER_H
#define PHIL_TEST_OUTPUTTER_H

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestListener.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/Test.h>
#include <cppunit/Outputter.h>


/**
 * I wrote this class because I was dissatisfied with the text runner of
 * CppUnit. In fact, except for the concept of running test suites, I am
 * dissatisfied with CppUnit. I find the overall architecture very complex,
 * and still not as helpful as I wish it were. This works with CppUnit 1.8.0
 * I don't think it work with any other version.
 *
 * The goal of PhilTestRunner was for me to make the output of the testing
 * more readable and useful. It has the following characteristics:
 *
 * - it prints the name of the suite being run and separate two suites with a
 *   blank line, so that you know better in which suite you are.
 *
 * - it prints the name of the test being run, so that when you have some kind
 *   of debug output, you know to which test it belongs.
 *
 * - the failures are reported in detail while the suite still runs, directly
 *   in the offending test. This differs from CppUnit which reports them at
 *   the end. This is very useful if you have test suites that take a long
 *   time. By placing the most offending test in the first position of the
 *   suite, you don't need to wait for the whole test to finish before
 *   spotting and understanding the failure.
 *
 * - on windows, the failures are also printed in the Debug window, using
 *   OutputDebugString. This output is in the Visual C++ debug format, so
 *   that pressing F4 (even during test run) sets your cursor directly on the
 *   right line, in the right file. Very useful!
 *
 * - it prints a short but nice summary at the end, with the number of test
 *   run, failed, and the percentage.
 *   
 * The output of a run looks like this:

--- suite TestFoo
        + testOne
        + testTwo
d:\software\cppunit-1.8.0\testunitexample\testfoo.cpp(40) : Failure!
Expected: 7, but was: 2.

        + testThree
d:\software\cppunit-1.8.0\testunitexample\testfoo.cpp(45) : Failure!
Uh uh, this test failed

--- suite TestMyClass

 *
 * All you need to use in this class is the PhilTestRunner. Ignore the other
 * classes, there are only used within PhilTestRunner. I package
 * everything in one header file and one source file to make distribution more
 * convenient.
 *
 * To use the PhilTestRunner, just do like with any other runner :

	PhilTestRunner runner;
	runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );
	return runner.run( "", false, true, true );

 *
 *
 * Send any comment, bug, suggestion, patch to phil@freehackers.org
 *
 */


/**
 * This class belongs to CppUnit. I hope they will include it!
 */
namespace CppUnit {

	class ExtendedTestRunner : public TextUi::TestRunner {

	public:
	ExtendedTestRunner( Outputter *outputter =NULL, TestListener * progressListener =NULL) 
		: TextUi::TestRunner( outputter), m_progressListener( progressListener)  {
		if (m_progressListener == NULL) {
			m_progressListener = new TextTestProgressListener();
		}
	}

  virtual bool runTest( Test *test,
	  bool doPrintProgress );
  
  virtual void setTestProgressListener( TestListener * listener) {
	  delete m_progressListener;
	  m_progressListener = listener;
  }

  virtual void setTestResultCollector( TestResultCollector * result) {
	  delete m_result;
	  m_result = result;
  }
  
	
protected:
	TestListener * m_progressListener;
};

}; // CppUnit


class PhilTestRunner : public CppUnit::ExtendedTestRunner {

public:
	/** I use all the Phil customisers in this runner */
	PhilTestRunner();
	
};



// -------------------------------------------------------------------
// You don't need the class after this line, they are used inside the
// PhilTestRunner.
//


class PhilProgressListener : public CppUnit::TestListener {

public:
	virtual ~PhilProgressListener () {}

	virtual void startTest (CppUnit::Test *test);
	virtual void addFailure (const CppUnit::TestFailure &failure);
	virtual void endTest (CppUnit::Test *test);

protected:
	std::string _currentSuite;
};



class PhilOutputter : public CppUnit::Outputter {
public:
	PhilOutputter( CppUnit::TestResultCollector * resultCollector )
		: m_resultCollector( resultCollector ) {}

	virtual ~PhilOutputter() {}
	void write();

protected:
	CppUnit::TestResultCollector * m_resultCollector;
};





#endif // PHIL_TEST_OUTPUTTER_H
