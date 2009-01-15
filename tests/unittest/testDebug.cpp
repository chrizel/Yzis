
#include "testDebug.h"

void TestYDebugBackend::testAreaLevel()
{
    YDebugBackend * dbe = YDebugBackend::self();
    dbe->clearArea();
    dbe->setDebugLevel( YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->debugLevel(), YZ_DEBUG_LEVEL );

    QString a1 = "area1", a2 = "area2";

    QCOMPARE( dbe->areaLevel(a1), YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->areaLevel(a2), YZ_DEBUG_LEVEL );

    dbe->setAreaLevel( a1, YZ_ERROR_LEVEL );
    QCOMPARE( dbe->areaLevel(a1), YZ_ERROR_LEVEL );
    QCOMPARE( dbe->areaLevel(a2), YZ_DEBUG_LEVEL );

    dbe->setDebugLevel( YZ_WARNING_LEVEL );
    QCOMPARE( dbe->areaLevel(a1), YZ_ERROR_LEVEL );
    QCOMPARE( dbe->areaLevel(a2), YZ_WARNING_LEVEL );

    dbe->removeArea( a1 );
    QCOMPARE( dbe->areaLevel(a1), YZ_WARNING_LEVEL );
    QCOMPARE( dbe->areaLevel(a2), YZ_WARNING_LEVEL );
}


void TestYDebugBackend::testSubAreaLevel()
{
    YDebugBackend * dbe = YDebugBackend::self();
    dbe->clearArea();
    dbe->setDebugLevel( YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->debugLevel(), YZ_DEBUG_LEVEL );

    QString a1 = "area1", a11 = "area1.1bis", a12 = "area1.2bis";

    QCOMPARE( dbe->areaLevel(a1), YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->areaLevel(a11), YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->areaLevel(a12), YZ_DEBUG_LEVEL );

    dbe->setAreaLevel( a1, YZ_ERROR_LEVEL );
    QCOMPARE( dbe->areaLevel(a1), YZ_ERROR_LEVEL );
    QCOMPARE( dbe->areaLevel(a11), YZ_ERROR_LEVEL );
    QCOMPARE( dbe->areaLevel(a12), YZ_ERROR_LEVEL );

    dbe->setAreaLevel( a11, YZ_WARNING_LEVEL );
    QCOMPARE( dbe->areaLevel(a1), YZ_ERROR_LEVEL );
    QCOMPARE( dbe->areaLevel(a11), YZ_WARNING_LEVEL );
    QCOMPARE( dbe->areaLevel(a12), YZ_ERROR_LEVEL );
}


void TestYDebugBackend::testSprintf()
{
    // segfault ?
    yzError().SPrintf( "%d %s %f", 34, "toto", 1.0 );
}

void TestYDebugBackend::testWhere()
{
    // need to check debug output to see that one
    yzError() << HERE() << endl;
    yzError() << HERE() << endl;
    yzError() << LOCATION() << endl;
    yzError() << LOCATION() << endl;
}

void TestYDebugBackend::testParseArgv()
{
    YDebugBackend * dbe = YDebugBackend::self();
    dbe->clearArea();
    dbe->setDebugLevel( YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->debugLevel(), YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->areaLevel("a"), YZ_DEBUG_LEVEL );

    const char * my_argv [10];
    int my_argc;

    my_argv[0] = "some_program";

    my_argv[1] = "--level=warning";
    my_argc = 2;
    dbe->parseArgv( my_argc, my_argv );
    QCOMPARE( dbe->debugLevel(), YZ_WARNING_LEVEL );
    QCOMPARE( dbe->areaLevel("a"), YZ_WARNING_LEVEL );

    my_argv[1] = "--level=debug";
    my_argv[2] = "--area-level=a,error";
    my_argc = 3;
    dbe->parseArgv( my_argc, my_argv );
    QCOMPARE( dbe->debugLevel(), YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->areaLevel("a"), YZ_ERROR_LEVEL );

    my_argv[1] = "--level=error";
    my_argv[2] = "--area-level=b,warning";
    my_argc = 3;
    dbe->parseArgv( my_argc, my_argv );
    QCOMPARE( dbe->debugLevel(), YZ_ERROR_LEVEL );
    QCOMPARE( dbe->areaLevel("b"), YZ_WARNING_LEVEL );

    my_argv[1] = "--level=fatal";
    my_argv[2] = "--area-level=b,debug";
    my_argc = 3;
    dbe->parseArgv( my_argc, my_argv );
    QCOMPARE( dbe->debugLevel(), YZ_FATAL_LEVEL );
    QCOMPARE( dbe->areaLevel("b"), YZ_DEBUG_LEVEL );


    my_argv[1] = "--level=error";
    my_argv[2] = "--area-level=c,deepdebug";
    my_argc = 3;
    dbe->parseArgv( my_argc, my_argv );
    QCOMPARE( dbe->debugLevel(), YZ_ERROR_LEVEL );
    QCOMPARE( dbe->areaLevel("c"), YZ_DEEPDEBUG_LEVEL );

    my_argv[1] = "--level=deepdebug";
    my_argc = 2;
    dbe->parseArgv( my_argc, my_argv );
    QCOMPARE( dbe->debugLevel(), YZ_DEEPDEBUG_LEVEL );
    QCOMPARE( dbe->areaLevel("b"), YZ_DEBUG_LEVEL );
}

void TestYDebugBackend::testParseArgv2()
{
    YDebugBackend * dbe = YDebugBackend::self();
    dbe->clearArea();
    dbe->setDebugLevel( YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->debugLevel(), YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->areaLevel("a"), YZ_DEBUG_LEVEL );

    QStringList argv;
    argv << "nyzis" << "--level=debug" << "--toto" << "titi" << "--area-level=b,warning";
    dbe->parseArgv( argv );
    QString s = argv.join("|");
    QCOMPARE( qp(s), "nyzis|--toto|titi" );
}




void TestYDebugBackend::testParseRcFile()
{
    YDebugBackend * dbe = YDebugBackend::self();
    dbe->clearArea();
    dbe->setDebugLevel( YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->debugLevel(), YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->areaLevel("a"), YZ_DEBUG_LEVEL );

    const char * fname = "yzdebugrc_test";
    QFile f(fname);
    QTextStream ts;

    // test start
    QVERIFY( f.open( QFile::ReadWrite ) );
    ts.setDevice(&f);
    ts << "   level: warning \n";
    ts << " a : error \n";
    f.close();
    QVERIFY( QFile::exists( fname ) );

    dbe->parseRcfile( fname );
    QCOMPARE( dbe->debugLevel(), YZ_WARNING_LEVEL );
    QCOMPARE( dbe->areaLevel("a"), YZ_ERROR_LEVEL );

    // test start
    QVERIFY( f.open( QFile::ReadWrite ) );
    ts.setDevice(&f);
    ts << "   level: error \n";
    ts << " a : debug \n";
    f.close();
    QVERIFY( QFile::exists( fname ) );

    dbe->parseRcfile( fname );
    QCOMPARE( dbe->debugLevel(), YZ_ERROR_LEVEL );
    QCOMPARE( dbe->areaLevel("a"), YZ_DEBUG_LEVEL );

    // test start
    QVERIFY( f.open( QFile::ReadWrite ) );
    ts.setDevice(&f);
    ts << "   level: debug \n";
    ts << " a : warning \n";
    ts << " CC : deepdebug \n";
    f.close();
    QVERIFY( QFile::exists( fname ) );

    dbe->parseRcfile( fname );
    QCOMPARE( dbe->debugLevel(), YZ_DEBUG_LEVEL );
    QCOMPARE( dbe->areaLevel("a"), YZ_WARNING_LEVEL );
    QCOMPARE( dbe->areaLevel("CC"), YZ_DEEPDEBUG_LEVEL );

    // test start
    QVERIFY( f.open( QFile::ReadWrite ) );
    ts.setDevice(&f);
    ts << "   level: deepdebug \n";
    ts << " a : warning \n";
    ts << " AA : error \n";
    f.close();
    QVERIFY( QFile::exists( fname ) );

    dbe->parseRcfile( fname );
    QCOMPARE( dbe->debugLevel(), YZ_DEEPDEBUG_LEVEL );
    QCOMPARE( dbe->areaLevel("a"), YZ_WARNING_LEVEL );
    QCOMPARE( dbe->areaLevel("AA"), YZ_ERROR_LEVEL );


    QFile::remove( fname );
}

#include "testDebug.moc"

