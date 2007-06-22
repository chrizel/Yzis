
#ifndef TEST_RESOURCE_H
#define TEST_RESOURCE_H

#include <QtTest/QtTest>

#include "resourcemgr.h"

class TestResource : public QObject
{
    Q_OBJECT

protected:
    bool mInitSuccess;
    YZResourceMgr * mResMgr;

protected:
    void TestResource::subTest( ResourceType restype, QString dir, QString fname, bool shouldFind);

private slots:
    // whole suite execution
    void initTestCase();
    void cleanupTestCase();

    // each test case execution
    void init();
    void cleanup();

    void testResourceMgrNewDel();
    void testYzisHomeEnv();
    void testResourceMgrCreatesYzisDir();
    void testUserScript();
    void testConfigScript();
    void testSyntaxFile();
    void testIndentFile();
    void testConfigFile();
    void testWConfigFile();

};

#endif // TEST_RESOURCE_H


