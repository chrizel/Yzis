/**
 * $Id: yz_buffer.cpp,v 1.25 2003/04/25 18:31:02 mikmak Exp $
 */

#include <cstdlib>
#include <qfile.h>
#include <qtextstream.h>
#include "yz_buffer.h"
#include "yz_events.h"
#include "yz_view.h"

YZBuffer::YZBuffer(QString _path) {
	path	= _path;
	view_nb	= 0;

	if (!_path.isEmpty()) load();
	view_list.setAutoDelete( true ); //we own views
}

void YZBuffer::addChar (int x, int y, QChar c) {
	/* brute force, we'll have events specific for that later on */
	QString l=findLine(y);
	if (l.isNull()) return;

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

void YZBuffer::addNewLine( int col, int line ) {
	QString l=findLine(line);
	if (l.isNull()) return;

	//replace old line
	text[ line ] = l.left( col );

	//add new line
	QString newline = l.mid( col );
	QStringList::Iterator it = text.at( line );
	text.insert( ++it, newline );
	/* inform the views */
	postEvent(mk_event_setline(line,&text[ line ]));
	postEvent(mk_event_setline(line+1,&newline));
}

void YZBuffer::postEvent(yz_event e) {
	for ( YZView *v = view_list.first();v;v=view_list.next() ) {
			v->postEvent(e);
	}
}

void YZBuffer::addView (YZView *v) {
	view_list.append( v );
	updateView(v);
}

//should be moved inside YZView
void YZBuffer::updateView(YZView *view) {
	for (int y=view->getCurrent(); y<text.count() && view->isLineVisible(y); y++) {
		QString l = findLine( view->getCurrent()+y );
		if (l.isNull()) continue;
		view->postEvent(mk_event_setline(y,&l));
	}
	view->updateCursor();
}

void YZBuffer::updateAllViews() {
	for ( YZView *v = view_list.first();v;v=view_list.next() ) updateView(v);
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

void YZBuffer::load() {
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

void YZBuffer::save() {
	if (path.isEmpty()) {
		//error("called though path is null, ignored");
		return;
	}
	QFile file( path );
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		for ( QStringList::Iterator it = text.begin(); it != text.end(); ++it )
			stream << *it << "\n";
		file.close();
	}
}


