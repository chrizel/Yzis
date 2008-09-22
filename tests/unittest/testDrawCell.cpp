#include "testDrawCell.h"

#include <libyzis/drawcell.h>

void TestDrawCell::testDrawCell()
{
	YDrawCell cell;
	foreach(QString c, QStringList()<<"h"<<"e"<<"l"<<"l"<<"o"<<"    "/*4*/<<"w"<<"o"<<"r"<<"l"<<"d") {
		cell.step(c);
	}
	QCOMPARE(cell.content(), QString("hello    world"));
	QCOMPARE(cell.width(), 14);
	QCOMPARE(cell.length(), 11);
	QCOMPARE(cell.widthForLength(7), 10);
	QCOMPARE(cell.lengthForWidth(10), 7);
}

