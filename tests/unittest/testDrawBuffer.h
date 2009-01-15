
#ifndef TEST_DRAWBUFFER_H
#define TEST_DRAWBUFFER_H

#include <QtTest/QtTest>

class TestDrawBuffer : public QObject
{
	Q_OBJECT

private slots:
	void testDrawLine();
	void testDrawBuffer();
	void testDrawBufferIterator();
	void testDrawBufferEmpty();

};

#endif // TEST_DRAWBUFFER_H

