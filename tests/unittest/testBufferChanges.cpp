
#include "testBufferChanges.h"

#include <QString>
#include <QStringList>

#include <libyzis/buffer.h>
#include <libyzis/cursor.h>
#include <libyzis/selection.h>

void TestBufferChanges::testBasic() {

	YBuffer b;
	b.setState(YBuffer::BufferActive);
	QCOMPARE(b.getWholeText(), QString(""));

	b.insertRegion(YCursor(0,0), YRawData() << "hello world");
	QCOMPARE(b.getWholeText(), QString("hello world\n"));

	b.insertRegion(YCursor(6,0), YRawData() << "" << "my name is ");
	QCOMPARE(b.getWholeText(), QString("hello \nmy name is world\n"));

	b.insertRegion(YCursor(11,1), YRawData() << "panard" << "this is a test" << "");
	QCOMPARE(b.getWholeText(), QString("hello \nmy name is panard\nthis is a test\nworld\n"));

	b.insertRegion(YCursor(0,3), YRawData() << "hello ");
	QCOMPARE(b.getWholeText(), QString("hello \nmy name is panard\nthis is a test\nhello world\n"));

	b.deleteRegion(YInterval(YCursor(5,2),YCursor(9,2)));
	QCOMPARE(b.getWholeText(), QString("hello \nmy name is panard\nthis test\nhello world\n"));

	b.deleteRegion(YInterval(YCursor(0,2),YBound(YCursor(0,3),true)));
	QCOMPARE(b.getWholeText(), QString("hello \nmy name is panard\nhello world\n"));

	b.deleteRegion(YInterval(YBound(YCursor(5,0),true),YBound(YCursor(6,2),true)));
	QCOMPARE(b.getWholeText(), QString("hello world\n"));

	b.insertRegion(YCursor(6,0), YRawData() << "" << "");
	QCOMPARE(b.getWholeText(), QString("hello \nworld\n"));

	b.insertRegion(YCursor(3,0), YRawData() << "" << "" << "" << "" );
	QCOMPARE(b.getWholeText(), QString("hel\n\n\nlo \nworld\n"));
}

