#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestSuite.h>
#include <set>


#if CPPUNIT_USE_TYPEINFO_NAME
#  include "cppunit/extensions/TypeInfoHelper.h"
#endif


namespace CppUnit {

/** (Implementation) This class manages all the TestFactoryRegistry.
 *
 * Responsible for the life-cycle of the TestFactoryRegistry.
 * 
 * TestFactory registry must call wasDestroyed() to indicate that
 * a given TestRegistry was destroyed, and needDestroy() to
 * know if a given TestFactory need to be destroyed (was not already
 * destroyed by another TestFactoryRegistry).
 */
class NamedRegistries
{
public:
  ~NamedRegistries();

  static NamedRegistries &getInstance();

  TestFactoryRegistry &getRegistry( std::string name );

  void wasDestroyed( TestFactory *factory );

  bool needDestroy( TestFactory *factory );

private:
  typedef std::map<std::string, TestFactoryRegistry *> Registries;
  Registries m_registries;

  typedef std::set<TestFactory *> Factories;
  Factories m_factoriesToDestroy;
  Factories m_destroyedFactories;
};


NamedRegistries::~NamedRegistries()
{
  Registries::iterator it = m_registries.begin();
  while ( it != m_registries.end() )
  {
    TestFactoryRegistry *registry = (it++)->second;
    if ( needDestroy( registry ) )
      delete registry;
  }
}


NamedRegistries &
NamedRegistries::getInstance()
{
  static NamedRegistries namedRegistries;
  return namedRegistries;
}


TestFactoryRegistry &
NamedRegistries::getRegistry( std::string name )
{
  Registries::const_iterator foundIt = m_registries.find( name );
  if ( foundIt == m_registries.end() )
  {
    TestFactoryRegistry *factory = new TestFactoryRegistry( name );
    m_registries.insert( std::make_pair( name, factory ) );
    m_factoriesToDestroy.insert( factory );
    return *factory;
  }
  return *foundIt->second;
}


void 
NamedRegistries::wasDestroyed( TestFactory *factory )
{
  m_factoriesToDestroy.erase( factory );
  m_destroyedFactories.insert( factory );
}


bool 
NamedRegistries::needDestroy( TestFactory *factory )
{
  return m_destroyedFactories.count( factory ) == 0;
}



TestFactoryRegistry::TestFactoryRegistry( std::string name ) :
    m_name( name )
{
}


TestFactoryRegistry::~TestFactoryRegistry()
{
  // The wasDestroyed() and needDestroy() is used to prevent
  // a double destruction of a factory registry.
  // registerFactory( "All Tests", getRegistry( "Unit Tests" ) );
  // => the TestFactoryRegistry "Unit Tests" is owned by both
  // the "All Tests" registry and the NamedRegistries...
  NamedRegistries::getInstance().wasDestroyed( this );

  for ( Factories::iterator it = m_factories.begin(); it != m_factories.end(); ++it )
  {
    TestFactory *factory = it->second;
    if ( NamedRegistries::getInstance().needDestroy( factory ) )
      delete factory;
  }
}


TestFactoryRegistry &
TestFactoryRegistry::getRegistry()
{
  return getRegistry( "All Tests" );
}


TestFactoryRegistry &
TestFactoryRegistry::getRegistry( const std::string &name )
{
  return NamedRegistries::getInstance().getRegistry( name );
}


void 
TestFactoryRegistry::registerFactory( const std::string &name,
                                      TestFactory *factory )
{
  m_factories[name] = factory;
}


void 
TestFactoryRegistry::registerFactory( TestFactory *factory )
{
    static int serialNumber = 1;

    OStringStream ost;
    ost << "@Dummy@" << serialNumber++;

    registerFactory( ost.str(), factory );
}


Test *
TestFactoryRegistry::makeTest()
{
  TestSuite *suite = new TestSuite( m_name );
  addTestToSuite( suite );
  return suite;
}


void 
TestFactoryRegistry::addTestToSuite( TestSuite *suite )
{
  for ( Factories::iterator it = m_factories.begin(); 
        it != m_factories.end(); 
        ++it )
  {
    TestFactory *factory = (*it).second;
    suite->addTest( factory->makeTest() );
  }
}


}  // namespace CppUnit
