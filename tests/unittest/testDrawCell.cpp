#include "testDrawCell.h"

#include <libyzis/drawcell.h>

void TestDrawCell::testDrawCell()
{
	YDrawCell cell, c2;
	foreach(QString c, QStringList()<<"h"<<"e"<<"l"<<"l"<<"o"<<"    "/*4*/<<"w"<<"o"<<"r"<<"l"<<"d") {
		cell.step(c);
	}
	QCOMPARE(cell.content(), QString("hello    world"));
	QCOMPARE(cell.width(), 14);
	QCOMPARE(cell.length(), 11);
	QCOMPARE(cell.widthForLength(7), 10);
	QCOMPARE(cell.lengthForWidth(10), 7);

	c2 = cell.left(6);
	QCOMPARE(c2.content(), QString("hello "));
	c2 = cell.mid(6);
	QCOMPARE(c2.content(), QString("   world"));
	QCOMPARE(c2.steps().count(), 5);
}

