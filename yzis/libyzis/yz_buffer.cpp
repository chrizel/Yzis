/**
 * $id$
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "yz_buffer.h"
#include "yz_events.h"
#include "yz_view.h"
#include <qfile.h>
#include <qtextstream.h>

YZBuffer::YZBuffer(QString _path) {
	path	= _path;
	view_nb	= 0;

	if (!_path.isEmpty()) load();
}

void YZBuffer::addChar (int x, int y, QChar c) {
	/* brute force, we'll have events specific for that later on */
	QString l=findLine(y);
	if (l.isNull()) return;

	/* do the actual modification */
	l.insert(x, c);

	text[y] = l;

	/* inform the views */
	postEvent(mk_event_setline(y,&l));
}

void YZBuffer::chgChar (int x, int y, QChar c) {
	/* brute force, we'll have events specific for that later on */
	QString l=findLine(y);
	if (l.isNull()) return;

	/* do the actual modification */
	l.remove(x, 1);
	l.insert(x, c);

	text[y] = l;

	/* inform the views */
	postEvent(mk_event_setline(y,&l));
}

void YZBuffer::postEvent(yz_event e) {
	int l = e.u.setline.y;

	/* quite basic, we just check that the line is currently displayed */
	for (int i=0; i<view_nb; i++) {
		YZView *v = view_list[i];
		if (v->isLineVisible(l))
			v->postEvent(e);
	}
}

void YZBuffer::addView (YZView *v) {
	view_list[view_nb] = v;
	updateView(view_nb++);
}

void YZBuffer::updateView(int view_nb) {
	int y;
	YZView *view = view_list[view_nb];

	for (y=view->getCurrent(); y<text.count() && view->isLineVisible(y); y++) {

		QString l = findLine( view->getCurrent()+y);
		if (l.isNull()) continue;
		view->postEvent(mk_event_setline(y,&l));
	}

	view->postEvent(mk_event_setcursor(0,0));
	
//MM opening files may be done in different modes
	//view->postEvent(mk_event_setstatus("Yzis Ready"));
}

void YZBuffer::updateAllViews() {
	for ( int i = 0 ; i < view_nb ; i++) updateView(i);
}

void  YZBuffer::addLine(QString &l) {
	text.append(l);
}

QString	YZBuffer::findLine(int line) {
	//we need to check this line exists.
	//the guy i talked with on IRC was right to doubt about it :)
	//so I return QString::null then for each call we need to check for if (!line.isNull())
	//trying if(!line) is NOT working (read QString doc)
	if ( text.count() <= line ) return QString::null;
	else return text[ line ];
}

void YZBuffer::load(void) {
	QFile file( path );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString line(stream.readLine() ); // line of text excluding '\n'
			addLine( line );
		}
		file.close();
	}
	updateAllViews();
}

void YZBuffer::save(void) {
	if (path.isEmpty()) {
		//error("called though path is null, ignored");
		return;
	}
}

