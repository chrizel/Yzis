
#include "testDrawBuffer.h"

#include <libyzis/drawbuffer.h>
#include <libyzis/color.h>

#define STRING_FROM_QSTRINGLIST(x, sl) \
	x = QString::number((sl).count())+"://"; \
	foreach( QString s, sl ) x += s + "#"
#define STRING_FROM_INTLIST(x, l) \
	x = QString::number((l).count())+"://"; \
	foreach( int i, l ) x += QString::number(i) + "#"

#define CHECK_STEPS(sA, sB) \
	STRING_FROM_INTLIST(a, sA); \
	STRING_FROM_INTLIST(b, sB); \
	QCOMPARE(a,b);

#define CHECK_CELLSCONTENT(dl, sl) \
	cC.clear(); \
	foreach( YDrawCell c, dl.cells() ) cC << c.c; \
	STRING_FROM_QSTRINGLIST(a, cC); \
	STRING_FROM_QSTRINGLIST(b, sl); \
	QCOMPARE(a, b);

void TestDrawBuffer::testDrawLine()
{
	QString a, b;
	QStringList cC;
	YDrawLine dl;
	YDrawSection ds;

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

	CHECK_CELLSCONTENT(dl, QStringList()<<"hel"<<"lo    w"<<"orld");
	CHECK_STEPS(dl.steps(),QList<int>()<<1<<1<<1<<1<<1<<4<<1<<1<<1<<1<<1);

	ds = dl.arrange(11);
	QCOMPARE(ds.count(), 2);
	CHECK_CELLSCONTENT(ds[0], QStringList()<<"hel"<<"lo    w"<<"o");
	CHECK_STEPS(ds[0].steps(), QList<int>()<<1<<1<<1<<1<<1<<4<<1<<1);
	CHECK_CELLSCONTENT(ds[1], QStringList()<<"rld");
	CHECK_STEPS(ds[1].steps(), QList<int>()<<1<<1<<1);

	ds = dl.arrange(7);
	QCOMPARE(ds.count(), 2);
	CHECK_CELLSCONTENT(ds[0], QStringList()<<"hel"<<"lo  ");
	CHECK_STEPS(ds[0].steps(), QList<int>()<<1<<1<<1<<1<<1<<2);
	CHECK_CELLSCONTENT(ds[1], QStringList()<<"  w"<<"orld");
	CHECK_STEPS(ds[1].steps(), QList<int>()<<2<<1<<1<<1<<1<<1);

	ds = dl.arrange(6);
	QCOMPARE(ds.count(), 3);
	CHECK_CELLSCONTENT(ds[0], QStringList()<<"hel"<<"lo ");
	CHECK_STEPS(ds[0].steps(), QList<int>()<<1<<1<<1<<1<<1<<1);
	CHECK_CELLSCONTENT(ds[1], QStringList()<<"   w"<<"or");
	CHECK_STEPS(ds[1].steps(), QList<int>()<<3<<1<<1<<1);
	CHECK_CELLSCONTENT(ds[2], QStringList()<<"ld");
	CHECK_STEPS(ds[2].steps(), QList<int>()<<1<<1);
}

void TestDrawBuffer::testDrawBuffer()
{
	// TODO
}

#define COMPARE_CELLINFO(ci, ci_t, ci_p, ci_cc) \
	QCOMPARE(ci.type, ci_t); \
	QCOMPARE(ci.pos, ci_p); \
	QCOMPARE(ci.cell.c, ci_cc)
#define NEXT_CELLINFO(ci, it) \
	QVERIFY(it.hasNext()); \
	ci = it.next()

void TestDrawBuffer::testDrawBufferIterator()
{
	YDrawBuffer db(11, 20);

	YDrawLine dl;
	dl.setColor(YColor("red"));
	dl.push("h"); dl.push("e"); dl.push("l");
	dl.setColor(YColor("green"));
	dl.push("l"); dl.push("o"); dl.push("    "/*4*/); dl.push("w");
	dl.setColor(YColor("red"));
	dl.push("o"); dl.push("r"); dl.push("l"); dl.push("d");
	dl.flush();

	db.setBufferDrawSection(0, dl.arrange(db.screenWidth()));
	/*drawbuffer content:
	 * 0:hel#lo    w#o#
	 *   rld#
	 */
	YDrawCellInfo ci;
	YDrawBufferIterator it = db.iterator(YInterval(YCursor(0,0), YBound(YCursor(0,2), true)));
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::Data, YCursor(0,0), QString("hel"));
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::Data, YCursor(3,0), QString("lo    w"));
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::Data, YCursor(10,0), QString("o"));
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::Data, YCursor(0,1), QString("rld"));
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::EOL, YCursor(3,1), QString(" "));
	QVERIFY(!it.hasNext());

	it = db.iterator(YInterval(YCursor(1,0), YCursor(1,0)));
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::Data, YCursor(1,0), QString("e"));
	QVERIFY(!it.hasNext());

	it = db.iterator(YInterval(YCursor(2,0), YCursor(4,0)));
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::Data, YCursor(2,0), QString("l"));
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::Data, YCursor(3,0), QString("lo"));
	QVERIFY(!it.hasNext());
}

void TestDrawBuffer::testDrawBufferEmpty()
{
	YDrawBuffer db(10, 10);
	YDrawCellInfo ci;
	YDrawBufferIterator it = db.iterator(YInterval(YCursor(0,0), YCursor(0,0)));
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::EOL, YCursor(0,0), QString(" "));
	QVERIFY(!it.hasNext());
}

