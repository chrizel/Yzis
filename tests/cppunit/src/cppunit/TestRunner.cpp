#include <cppunit/TestSuite.h>
#include <cppunit/TextTestResult.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/TestResult.h>
#include <cppunit/ui/text/TestRunner.h>
#include <iostream>


namespace CppUnit {
namespace TextUi {

/*! Constructs a new text runner.
 * \param outputter used to print text result. Owned by the runner.
 */
TestRunner::TestRunner( Outputter *outputter ) 
    : m_outputter( outputter )
    , m_suite( new TestSuite( "All Tests" ) )
    , m_result( new TestResultCollector() )
    , m_eventManager( new TestResult() )
{
  if ( !m_outputter )
    m_outputter = new TextOutputter( m_result, std::cout );
  m_eventManager->addListener( m_result );
}


TestRunner::~TestRunner()
{
  delete m_eventManager;
  delete m_outputter;
  delete m_result;
  delete m_suite;
}


/*! Adds the specified test.
 *
 * \param test Test to add.
 */
void 
TestRunner::addTest( Test *test )
{
  if ( test != NULL )
    m_suite->addTest( test );
}


/*! Runs the named test case.
 *
 * \param testName Name of the test case to run. If an empty is given, then
 *                 all added test are run. The name must be the name of
 *                 of an added test.
 * \param doWait if \c true then the user must press the RETURN key 
 *               before the run() method exit.
 * \param doPrintResult if \c true (default) then the test result are printed
 *                      on the standard output.
 * \param doPrintProgress if \c true (default) then TextTestProgressListener is
 *                        used to show the progress.
 * \return \c true is the test was successful, \c false if the test
 *         failed or was not found.
 */
bool
TestRunner::run( std::string testName,
                     bool doWait,
                     bool doPrintResult,
                     bool doPrintProgress )
{
  runTestByName( testName, doPrintProgress );
  printResult( doPrintResult );
  wait( doWait );
  return m_result->wasSuccessful();
}


bool
TestRunner::runTestByName( std::string testName,
                               bool doPrintProgress )
{
  if ( testName.empty() )
    return runTest( m_suite, doPrintProgress );

  Test *test = findTestByName( testName );
  if ( test != NULL )
    return runTest( test, doPrintProgress );

  std::cout << "Test " << testName << " not found." << std::endl;
  return false;
}


void 
TestRunner::wait( bool doWait )
{
  if ( doWait ) 
  {
    std::cout << "<RETURN> to continue" << std::endl;
    std::cin.get ();
  }
}


void 
TestRunner::printResult( bool doPrintResult )
{
  std::cout << std::endl;
  if ( doPrintResult )
    m_outputter->write();
}


Test * findTestOfSuite( std::string name, TestSuite * suite )
{
  if (suite->getName() == name) return suite;
/*
  std::cout << "findTestOfSuite: looking for '" << name << "' in '" 
  		<< suite->getName() << "'" << std::endl;
*/
  if (suite->getName().find( name ) != std::string::npos) return suite;
  for ( std::vector<Test *>::const_iterator it = suite->getTests().begin(); 
        it != suite->getTests().end(); 
        ++it )
  {
    Test *test = *it;
    TestSuite * childSuite;
    if (test->toString().find( "suite" ) != std::string::npos) {
      childSuite = (TestSuite *) test;
      test = findTestOfSuite( name, childSuite );
      if (test != NULL) return test;
    } else {
//      std::cout << "Trying test '" << test->getName() << "'" << std::endl;
      if ( test->getName() == name )
        return test;
      }
  }
  return NULL;

}

Test * 
TestRunner::findTestByName( std::string name ) const
{
  return findTestOfSuite( name, m_suite );
}


bool
TestRunner::runTest( Test *test,
                         bool doPrintProgress )
{
  TextTestProgressListener progress;
  if ( doPrintProgress )
    m_eventManager->addListener( &progress );

  test->run( m_eventManager );

  if ( doPrintProgress )
    m_eventManager->removeListener( &progress );
  return m_result->wasSuccessful();
}


/*! Returns the result of the test run.
 * Use this after calling run() to access the result of the test run.
 */
TestResultCollector &
TestRunner::result() const
{
  return *m_result;
}


/*! Returns the event manager.
 * The instance of TestResult results returned is the one that is used to run the
 * test. Use this to register additional TestListener before running the tests.
 */
TestResult &
TestRunner::eventManager() const
{
  return *m_eventManager;
}


/*! Specifies an alternate outputter.
 *
 * Notes that the outputter will be use after the test run only if \a printResult was
 * \c true.
 * \see CompilerOutputter, XmlOutputter, TextOutputter.
 */
void 
TestRunner::setOutputter( Outputter *outputter )
{
  delete m_outputter;
  m_outputter = outputter;
}


} // namespace TextUi
}  //  namespace CppUnit
