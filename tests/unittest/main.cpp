
#include "testDebug.h"
#include "testColor.h"

int main( int argc, char * argv[] )
{
    TestYZDebugBackend testYZDebugBackend;
    QTest::qExec( &testYZDebugBackend, argc, argv );

    printf("\n");

    TestColor testColor;
    QTest::qExec( &testColor, argc, argv );
}

