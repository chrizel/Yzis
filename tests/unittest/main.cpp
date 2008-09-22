
#include "testDebug.h"
#include "testColor.h"
#include "testResource.h"
#include "testBufferChanges.h"
#include "testDrawCell.h"

#include <QRegExp>

/** Convert a QString list to an char * argv[] array.
  *
  * both argc and argv are updated. There is no check on the size
  * of argv.
  */
void toArgvArray( QStringList l, char ** argv, int * argc )
{
    *argc = 0;
    foreach( QString s, l ) {
        argv[(*argc)++] = strdup( qPrintable(s) );
    }
}

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
    // If you want to add a new test, just ignore the stuff here until the end
    // of main.
    // =======================[ stuff to ignore ]=====================
    QMap<QString, QString> runMe;
    QStringList myArgv;
    bool runAll = false;
    int result = 0, fakeArgc = 1;
    char * fakeArgv[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    QRegExp reTestName( "(\\w+)(::(\\w+))?(\\(\\))?" );

    myArgv << "yzistest.exe";

    if (argc == 1) {
        runAll = true;
    } else {
        for ( int i = 1; i < argc; i++) {
            //qDebug("argv[%d] = '%s'", i, argv[i] );
            if (argv[i][0] == '-') {
                myArgv << argv[i];
            } else if (reTestName.exactMatch( argv[i] )) {
                //qDebug("match: %s", qPrintable( reTestName.capturedTexts().join(" , ") ) );
                runMe[reTestName.cap(1)] = reTestName.cap(3);
            }
        }
    }

    QStringList myArgvBase = myArgv;

#define RUN_MY_TEST( TestName ) \
    myArgv = myArgvBase; \
    if (runAll || runMe.contains( #TestName )) { \
        foreach( QString key, runMe.keys() ) { \
            /* qDebug( "TestName=%s, key=%s, runMe[key]=%s\n", #TestName, * qp(key), qp(runMe[key]) ); */ \
            if (key == #TestName && (! runMe[key].isEmpty())) { \
                myArgv << runMe[key]; \
            } \
        } \
        \
        toArgvArray( myArgv, fakeArgv, &fakeArgc ); \
        fakeArgv[fakeArgc] = NULL; \
        \
        TestName TestName##inst ; \
        result += QTest::qExec( & TestName##inst, fakeArgc, fakeArgv ); \
        printf("\n"); \
    }

    // ===============================================================


    // Add your test here. You need to include it first at the top of the
    // main.
    RUN_MY_TEST( TestYDebugBackend )
    RUN_MY_TEST( TestColor )
    RUN_MY_TEST( TestResource )
    //RUN_MY_TEST( TestBufferChanges )
	RUN_MY_TEST( TestDrawCell )

    printf("Unittest status: %d failed tests\n", result );

    return result;
}

