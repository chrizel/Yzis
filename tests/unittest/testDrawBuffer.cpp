
#include "testDrawBuffer.h"

#include <libyzis/drawbuffer.h>
#include <libyzis/color.h>

#define STRING_FROM_QSTRINGLIST(x, sl) \
	x = QString::number((sl).count())+"://"; \
	foreach( QString s, sl ) x += s + "#"
#define STRING_FROM_INTLIST(x, l) \
	x = QString::number((l).count())+"://"; \
	foreach( int i, l ) x += QString::number(i) + "#"

#define CHECK_CELLSCONTENT(dl, sl) \
	cC.clear(); \
	foreach( YDrawCell c, dl ) cC << c.content(); \
	STRING_FROM_QSTRINGLIST(a, cC); \
	STRING_FROM_QSTRINGLIST(b, sl); \
	QCOMPARE(a, b);

#define CHECK_STEPS(dl, ss) \
	cI.clear(); \
	foreach( YDrawCell c, dl ) \
		foreach( int s, c.steps() ) cI << s;\
	STRING_FROM_INTLIST(a, cI); \
	STRING_FROM_INTLIST(b, ss); \
	QCOMPARE(a, b);


void TestDrawBuffer::testDrawLine()
{
	QString a, b;
	QStringList cC;
	QList<int> cI;
	YDrawLine dl;
	YDrawSection ds;

	dl.setColor(YColor("red"));
	dl.step("h");
	dl.step("e");
	dl.step("l");
	dl.setColor(YColor("green"));
	dl.step("l");
	dl.step("o");
	dl.step("    "); // 4
	dl.step("w");
	dl.setColor(YColor("red"));
	dl.step("o");
	dl.step("r");
	dl.step("l");
	dl.step("d");
	dl.flush();

	CHECK_CELLSCONTENT(dl, QStringList()<<"hel"<<"lo    w"<<"orld");
	CHECK_STEPS(dl, QList<int>()<<1<<1<<1<<1<<1<<4<<1<<1<<1<<1<<1);

	ds = dl.arrange(11);
	QCOMPARE(ds.count(), 2);
	CHECK_CELLSCONTENT(ds[0], QStringList()<<"hel"<<"lo    w"<<"o");
	CHECK_STEPS(ds[0], QList<int>()<<1<<1<<1<<1<<1<<4<<1<<1);
	CHECK_CELLSCONTENT(ds[1], QStringList()<<"rld");
	CHECK_STEPS(ds[1], QList<int>()<<1<<1<<1);

	ds = dl.arrange(7);
	QCOMPARE(ds.count(), 2);
	CHECK_CELLSCONTENT(ds[0], QStringList()<<"hel"<<"lo  ");
	CHECK_STEPS(ds[0], QList<int>()<<1<<1<<1<<1<<1<<2);
	CHECK_CELLSCONTENT(ds[1], QStringList()<<"  w"<<"orld");
	CHECK_STEPS(ds[1], QList<int>()<<1<<1<<1<<1<<1);
	QCOMPARE(ds[1][0].stepsShift(), 2);

	ds = dl.arrange(6);
	QCOMPARE(ds.count(), 3);
	CHECK_CELLSCONTENT(ds[0], QStringList()<<"hel"<<"lo ");
	CHECK_STEPS(ds[0], QList<int>()<<1<<1<<1<<1<<1<<1);
	CHECK_CELLSCONTENT(ds[1], QStringList()<<"   w"<<"or");
	CHECK_STEPS(ds[1], QList<int>()<<1<<1<<1);
	QCOMPARE(ds[1][0].stepsShift(), 3);
	CHECK_CELLSCONTENT(ds[2], QStringList()<<"ld");
	CHECK_STEPS(ds[2], QList<int>()<<1<<1);
}

void TestDrawBuffer::testDrawBuffer()
{
	// TODO
}

#define COMPARE_CELLINFO(ci, ci_t, ci_p, ci_cc) \
	QCOMPARE(ci.type, ci_t); \
	QCOMPARE(ci.pos, ci_p); \
	QCOMPARE(ci.cell.content(), ci_cc)
#define NEXT_CELLINFO(ci, it) \
	QVERIFY(it.isValid()); \
	ci = it.drawCellInfo(); \
	it.next()

void TestDrawBuffer::testDrawBufferIterator()
{
	YDrawBuffer db(NULL, 11, 20);

	YDrawLine dl;
	dl.setColor(YColor("red"));
	dl.step("h"); dl.step("e"); dl.step("l");
	dl.setColor(YColor("green"));
	dl.step("l"); dl.step("o"); dl.step("    "/*4*/); dl.step("w");
	dl.setColor(YColor("red"));
	dl.step("o"); dl.step("r"); dl.step("l"); dl.step("d");
	dl.flush();

	db.setBufferDrawSection(0, dl.arrange(db.screenWidth()));
	/*drawbuffer content:
	 * 0:hel#lo    w#o#
	 *   rld#
	 */
	YDrawCellInfo ci;
	YDrawBufferConstIterator it = db.const_iterator(YInterval(YCursor(0,0), YBound(YCursor(0,2), true)), yzis::ScreenInterval);
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
	QVERIFY(!it.isValid());

	it = db.const_iterator(YInterval(YCursor(1,0), YCursor(1,0)), yzis::ScreenInterval);
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::Data, YCursor(1,0), QString("e"));
	QVERIFY(!it.isValid());

	it = db.const_iterator(YInterval(YCursor(2,0), YCursor(4,0)), yzis::ScreenInterval);
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::Data, YCursor(2,0), QString("l"));
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::Data, YCursor(3,0), QString("lo"));
	QVERIFY(!it.isValid());
}

void TestDrawBuffer::testDrawBufferEmpty()
{
	YDrawBuffer db(NULL, 10, 10);
	YDrawCellInfo ci;
	YDrawBufferConstIterator it = db.const_iterator(YInterval(YCursor(0,0), YCursor(0,0)), yzis::ScreenInterval);
	NEXT_CELLINFO(ci, it);
	COMPARE_CELLINFO(ci, YDrawCellInfo::EOL, YCursor(0,0), QString(" "));
	QVERIFY(!it.isValid());
}

