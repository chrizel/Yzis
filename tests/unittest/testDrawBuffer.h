
#ifndef TEST_DRAWBUFFER_H
#define TEST_DRAWBUFFER_U

#include <QtTest/QtTest>

#include <QList>
#include <QStringList>

class YDrawLine;

class TestDrawBuffer : public QObject
{
	Q_OBJECT

private slots:
	void testDrawLine();
	void testDrawBuffer();

};

#endif


