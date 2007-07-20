
#include "testDebug.h"
#include "testColor.h"
#include "testResource.h"

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
    QMap<QString,QString> runMe;
    QStringList myArgv;
    bool runAll = false;
    int result=0,fakeArgc = 1;
    char * fakeArgv[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    QRegExp reTestName( "(\\w+)(::(\\w+))?(\\(\\))?" );

    myArgv << "yzistest.exe";

    if (argc == 1) {
        runAll = true;
    } else {
        for( int i=1; i<argc; i++) {
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
    } \

    RUN_MY_TEST( TestYZDebugBackend )
    RUN_MY_TEST( TestColor )
    RUN_MY_TEST( TestResource )

    return result;
}

