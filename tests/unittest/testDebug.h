
#ifndef TEST_DEBUG_H
#define TEST_DEBUG_H

#include <QtTest/QtTest>

#include "debug.h"

class TestYZDebugBackend: public QObject
{
    Q_OBJECT

private slots:
    void testAreaLevel();
    void testSubAreaLevel();
    void testSprintf();
    void testWhere();
    void testParseArgv();
    void testParseArgv2();
    void testParseRcFile();
};

#endif // TEST_DEBUG_H
