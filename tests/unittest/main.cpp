
#include "testDebug.h"
#include "testColor.h"

/** 
  * How to use it:
  *
  * Run an individual class test: yzistest <TestClassName>
  * Run all unit tests: yzistest
  *
  * To write a test, see the Qt documentation. 
  *
  */
int main( int argc, char * argv[] )
{
    QMap<QString,bool> runMe;
    bool runAll = false;
    int fakeArgc = 1;
    char * fakeArgv[] = { "yzistest.exe", NULL };

    if (argc == 1) {
        runAll = true;
    } else {
        for( int i=1; i<argc; i++) {
            runMe[argv[i]] = true;
        }
    }

    if (runAll || runMe.contains("TestYZDebugBackend")) {
        TestYZDebugBackend testYZDebugBackend;
        QTest::qExec( &testYZDebugBackend, fakeArgc, fakeArgv );
        printf("\n");
    }

    if (runAll || runMe.contains("TestColor")) {
        TestColor testColor;
        QTest::qExec( &testColor, fakeArgc, fakeArgv );
        printf("\n");
    }
}

