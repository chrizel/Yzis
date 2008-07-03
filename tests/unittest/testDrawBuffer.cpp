
#include "testDrawBuffer.h"

#include <libyzis/drawbuffer.h>
#include <libyzis/color.h>

#define STRING_FROM_QSTRINGLIST(x, sl) \
	x = QString::number(sl.count())+"://"; \
	foreach( QString s, sl ) x += s + "#"
#define STRING_FROM_INTLIST(x, l) \
	x = QString::number(l.count())+"://"; \
	foreach( int i, l ) x += QString::number(i) + "#"

void TestDrawBuffer::checkCellsContent( const YDrawLine& dl, const QStringList& sl )
{
	QStringList cC;
	foreach( YDrawCell c, dl.cells() ) cC << c.c;
	QString a, b;
	STRING_FROM_QSTRINGLIST(a, cC);
	STRING_FROM_QSTRINGLIST(b, sl);
	QCOMPARE(a, b);
}
void TestDrawBuffer::checkSteps( const YDrawLine& dl, const QList<int>& steps )
{
	QString a, b;
	STRING_FROM_INTLIST(a, dl.steps());
	STRING_FROM_INTLIST(b, steps);
	QCOMPARE(a,b);
}

void TestDrawBuffer::testDrawLine()
{
	YDrawLine dl;
	dl.setColor(YColor("red"));
	dl.push("h");
	dl.push("e");
	dl.push("l");
	dl.setColor(YColor("green"));
	dl.push("l");
	dl.push("o");
	dl.push("    "); // 4
	dl.push("w");
	dl.setColor(YColor("red"));
	dl.push("o");
	dl.push("r");
	dl.push("l");
	dl.push("d");
	dl.flush();

	checkCellsContent(dl, QStringList() << "hel" << "lo    w" << "orld");
	checkSteps(dl, QList<int>() << 1 << 1 << 1 << 1 << 1 << 4 << 1 << 1 << 1 << 1 << 1 );
}

void TestDrawBuffer::testDrawBuffer()
{
	// TODO
}

