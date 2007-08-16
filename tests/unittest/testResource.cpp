
#include "libyzis/session.h"

#include "testResource.h"
#include "resourcemgr.h"
#include "debug.h"

#include <QDir>
#include <QStringList>
#include <QFileInfo>

#include <iostream>
#include <stdlib.h>

using namespace std;

/** Remove a complete non empty directory tree.
  *
  * Path can be a directory or a file. If path does not exist, it is
  * considered already removed, so the function returns true.
  *
  * @return false when a dir or file can not be removed and true if the tree
  * is removed or if the path does not exist.
  */
bool removeTree( const QString & path )
{
    QStringList stack;
    QString f;
    QFileInfo fi;
    QStringList ls;

    if (!QFile::exists(path)) return true;

    stack.append( QFileInfo(path).absoluteFilePath() );

    while ( ! stack.isEmpty()) {
        //cout << "Stack: " << qp(stack.join(" , ")) << endl;
        f = stack.takeLast();
        //cout << "f=" << qp(f) << endl;
        fi.setFile(f);
        if (fi.isFile()) {
            //cout << "Removing file " << qp(f) << endl;
            if (! fi.dir().remove( f )) return false;
        } else {
            ls = QDir(f).entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files );
            //cout << "dir content: " << qp(ls.join(" , " ) ) << endl;
            if (ls.isEmpty()) {
                //cout << "Removing empty dir " << qp(f) << endl;
                if (! fi.dir().rmdir( f )) return false;
            } else {
                stack << f;
                foreach( QString fname, ls ) {
                    stack << QFileInfo(f + "/" + fname).absoluteFilePath();
                }
            }
        }
    }
    return true;
}

/** Create a file at the give @p path with the given @p content.
  *
  * If a file already exists, its content is overwritten with @p content.
  *
  * @return true on success
  */
bool createFile( QString path, QString content = QString() )
{
    FILE * f = fopen( path.toUtf8(), "w" );
    if (! f) return false;
    fprintf( f, "%s", qp(content) );
    fclose( f );
    return true;
}

void TestResource::initTestCase()
{
    mInitSuccess = true;
    QString yzisDir = QDir::homePath() + "/.yzis/";
    QString saveYzisDir = QDir::homePath() + "/.yzis_saved_for_tests/";

    QVERIFY( removeTree( saveYzisDir ) );

    if (QFile::exists( saveYzisDir )) {
        mInitSuccess = false;
        QFAIL( QString("Can not save .yzis to " + saveYzisDir + ". Aborting test.").toUtf8() );
        return ;
    }

    if (QFile::exists( yzisDir ) ) {
        // protect .yzis by saving it
        QDir d( yzisDir );
        d.cdUp();
        QVERIFY( (mInitSuccess = d.rename( yzisDir, saveYzisDir )) );
    }

}

void TestResource::cleanupTestCase()
{
    QVERIFY( mInitSuccess );
    QString yzisDir = QDir::homePath() + "/.yzis/";
    QString saveYzisDir = QDir::homePath() + "/.yzis_saved_for_tests/";

    QVERIFY( removeTree( yzisDir ) );

    // restore .yzis
    if (!QFile::exists( saveYzisDir ) ) {
        // nothing to do
        return ;
    }

    // protect .yzis by saving it
    QDir d( yzisDir );
    d.cdUp();
    QVERIFY( d.rename( saveYzisDir, yzisDir ) );
}

void TestResource::init()
{
    QVERIFY( removeTree( "./tmpfiles" ) );
    QVERIFY( removeTree( QDir::homePath() + "/.yzis" ) );
    QVERIFY( removeTree( "./yzishome" ) );
    QDir d = QDir::current();
    QVERIFY( d.mkdir( "tmpfiles" ) );
    QVERIFY( d.mkdir( "yzishome" ) );
    QVERIFY( d.cd("yzishome" ) );

    QCOMPARE( putenv( "YZISHOME=./yzishome"), 0 );

    mResMgr = new YResourceMgr();
}

void TestResource::cleanup()
{
    if (mResMgr != NULL) delete mResMgr;
    QVERIFY( removeTree( QDir::homePath() + "/.yzis" ) );
    QVERIFY( removeTree( "./yzishome" ) );
    QVERIFY( removeTree( "./tmpfiles" ) );
}

void TestResource::testResourceMgrNewDel()
{
    QVERIFY( mInitSuccess );

    QVERIFY( (mResMgr != NULL) );
    delete mResMgr;

    mResMgr = NULL;
    mResMgr = new YResourceMgr();
    QVERIFY( (mResMgr != NULL) );
    delete mResMgr;
    mResMgr = NULL;

    mResMgr = new YResourceMgr();
    QVERIFY( (mResMgr != NULL) );
    delete mResMgr;
    mResMgr = NULL;
}

void TestResource::testResourceMgrCreatesYzisDir()
{
    QVERIFY( mInitSuccess );

    // resource manager is alrady created, remove it
    delete mResMgr;
    mResMgr = NULL;
    QVERIFY( removeTree( QDir::homePath() + "/.yzis" ) );

    QVERIFY( ! QFile::exists( QDir::homePath() + "/.yzis" ) );
    mResMgr = new YResourceMgr();
    QVERIFY( (mResMgr != NULL) );

    QVERIFY( QFile::exists( QDir::homePath() + "/.yzis" ) );
}

void TestResource::testYzisHomeEnv()
{
    QVERIFY( mInitSuccess );
    char * yzisHome = getenv("YZISHOME");
    QVERIFY( ! (yzisHome == NULL) );
    QCOMPARE( yzisHome, "./yzishome" );
}

void TestResource::subTest( ResourceType restype, QString dir, QString fname, bool shouldFind)
{
    QString target;
    QString resource;

    target = QFileInfo( dir + fname ).absoluteFilePath();
    QVERIFY( createFile( target ) );
    resource = mResMgr->findResource( restype, fname );
    if (shouldFind) {
        if (! resource.isEmpty()) resource = QFileInfo(resource).absoluteFilePath();
        QCOMPARE( resource, target );
    } else {
        QCOMPARE( resource, QString() );
    }
}


void TestResource::testUserScript()
{
    QVERIFY( mInitSuccess );

    QDir d = QDir::home();
    QVERIFY( d.cd(".yzis") );
    QVERIFY( d.mkdir("scripts") );
    d = QDir( "yzishome" );
    QVERIFY( d.mkdir("scripts") );

    QString resource = mResMgr->findResource( UserScriptResource, "f1.lua" );
    QCOMPARE( resource, QString() );

    subTest( UserScriptResource, "yzishome/scripts/", "f1.lua", true );
    subTest( UserScriptResource, QDir::homePath() + "/.yzis/scripts/", "f1.lua", true );
    subTest( UserScriptResource, "", "tmpfiles/f1.lua", true );
    subTest( UserScriptResource, "", QFileInfo("tmpfiles/f2.lua").absoluteFilePath() , true );

}


// syntax file is not looked up reolative or absolute

void TestResource::testConfigScript()
{
    QVERIFY( mInitSuccess );

    QDir d = QDir::home();
    QVERIFY( d.cd(".yzis") );
    QVERIFY( d.mkdir("scripts") );
    d = QDir( "yzishome" );
    QVERIFY( d.mkdir("scripts") );

    QString resource = mResMgr->findResource( ConfigScriptResource, "f1.lua" );
    QVERIFY( resource.isEmpty() );

    subTest( ConfigScriptResource, "yzishome/scripts/", "f1.lua", true );
    subTest( ConfigScriptResource, QDir::homePath() + "/.yzis/scripts/", "f1.lua", true );
    subTest( ConfigScriptResource, "", "tmpfiles/f1.lua", false );
    subTest( ConfigScriptResource, "", QFileInfo("tmpfiles/f2.lua").absoluteFilePath() , true );

}

void TestResource::testSyntaxFile()
{
    QVERIFY( mInitSuccess );

    QDir d = QDir::home();
    QVERIFY( d.cd(".yzis") );
    QVERIFY( d.mkdir("syntax") );
    d = QDir( "yzishome" );
    QVERIFY( d.mkdir("syntax") );

    QString resource = mResMgr->findResource( SyntaxHlResource, "f1.xml" );
    QVERIFY( resource.isEmpty() );

    subTest( SyntaxHlResource, "yzishome/syntax/", "f1.xml", true );
    subTest( SyntaxHlResource, QDir::homePath() + "/.yzis/syntax/", "f1.xml", true );
    subTest( SyntaxHlResource, "", "tmpfiles/f1.xml", false );
    subTest( SyntaxHlResource, "", QFileInfo("tmpfiles/f2.xml").absoluteFilePath() , true );

}

void TestResource::testIndentFile()
{
    QVERIFY( mInitSuccess );

    QDir d = QDir::home();
    QVERIFY( d.cd(".yzis") );
    QVERIFY( d.mkdir("indent") );
    d = QDir( "yzishome" );
    QVERIFY( d.mkdir("indent") );

    QString resource = mResMgr->findResource( IndentResource, "f1.lua" );
    QVERIFY( resource.isEmpty() );

    subTest( IndentResource, "yzishome/indent/", "f1.lua", true );
    subTest( IndentResource, QDir::homePath() + "/.yzis/indent/", "f1.lua", true );
    subTest( IndentResource, "", "tmpfiles/f1.lua", false );
    subTest( IndentResource, "", QFileInfo("tmpfiles/f2.lua").absoluteFilePath() , true );

}

void TestResource::testConfigFile()
{
    QVERIFY( mInitSuccess );

    QDir d = QDir::home();

    QString resource = mResMgr->findResource( IndentResource, "f1.conf" );
    QVERIFY( resource.isEmpty() );

    subTest( ConfigResource, "yzishome/", "f1.conf", true );
    subTest( ConfigResource, QDir::homePath() + "/.yzis/", "f1.conf", true );
    subTest( ConfigResource, "", "tmpfiles/f1.conf", false );
    subTest( ConfigResource, "", QFileInfo("tmpfiles/f2.conf").absoluteFilePath() , true );


}

void TestResource::testWConfigFile()
{
    QVERIFY( mInitSuccess );
    QString target;
    QString resource, fname, expected;

    QDir d = QDir::home();
    QVERIFY( d.cd(".yzis") );
    d = QDir( "yzishome" );


    // create file in yzishome, .yzis is returned
    fname = "f1.conf";
    expected = QFileInfo( QDir::homePath() + "/.yzis/" + fname).absoluteFilePath();

    QVERIFY( createFile( QFileInfo( "yzishome/" + fname ).absoluteFilePath() ) );
    resource = mResMgr->findResource( WritableConfigResource, fname );
    QCOMPARE( resource, expected );

    // create absolute file , .yzis directory
    target = QFileInfo( "tmpfiles/" + fname).absoluteFilePath();
    QVERIFY( createFile( target ) );
    resource = mResMgr->findResource( WritableConfigResource, fname );
    QCOMPARE( resource, expected );

    expected = QFileInfo( QDir::homePath() + "/.yzis/tmpfiles/" + fname).absoluteFilePath();
    QVERIFY( createFile( QFileInfo( "tmpfiles/" + fname ).absoluteFilePath() ) );
    resource = mResMgr->findResource( WritableConfigResource, "tmpfiles/" + fname );
    QCOMPARE( resource, expected );

}
