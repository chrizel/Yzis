/**
 * $Id$
 */

#include "yz_events.h"
#include <cstdlib>
#include <qfile.h>
#include <qtextstream.h>
#include "yz_buffer.h"
#include "yz_view.h"
#include "yzis.h"
#include "yz_debug.h"
#include <assert.h>

YZBuffer::YZBuffer(YZSession *sess, const QString& _path) {
	session = sess;
	path	= _path;

	if (!_path.isEmpty()) load();
	else {
		QString blah( "" );
		addLine(blah);
	}
	//view_list.setAutoDelete( true ); //we own views
	session->addBuffer(this );
}

YZBuffer::~YZBuffer() {
//	view_list.clear();
	text.clear();
	//delete path;
}

void YZBuffer::addChar (int x, int y, const QString& c) {
	/* brute force, we'll have events specific for that later on */
	QString l=findLine(y);
	if (l.isNull()) return;

	l.insert(x, c);

	text[y] = l;

	/* inform the views */
	for ( YZView *v = view_list.first();v;v=view_list.next() ) {
			session->postEvent(YZEvent::mkEventInvalidateLine( v->myId,y ));
	}
#if 0
	yzDebug() << "list : " << view_list.count() << endl;
	for ( QMap<int,YZView*>::iterator it=view_list.begin(); it!=view_list.end(); ++it ) {
			YZView *v = static_cast<YZView *>( it.data() );
			assert( v );
			yzDebug() << "myId :" << v->myId << endl;
			session->postEvent(YZEvent::mkEventInvalidateLine( v->myId,y ));
	}
#endif
}

void YZBuffer::chgChar (int x, int y, const QString& c) {
	/* brute force, we'll have events specific for that later on */
	QString l=findLine(y);
	if (l.isNull()) return;

	/* do the actual modification */
	l.remove(x, 1);
	l.insert(x, c);

	text[y] = l;

	/* inform the views */
	for ( YZView *v = view_list.first();v;v=view_list.next() ) {
			session->postEvent(YZEvent::mkEventInvalidateLine( v->myId,y ));
	}
#if 0
	for ( QMap<int,YZView*>::iterator it=view_list.begin(); it!=view_list.end(); it++ ) {
			YZView *v = it.data();
			session->postEvent(YZEvent::mkEventInvalidateLine( v->myId, y ));
	}
#endif
}

void YZBuffer::delChar (int x, int y, int count) {
	/* brute force, we'll have events specific for that later on */
	QString l=findLine(y);
	if (l.isNull()) return;

	/* do the actual modification */
	l.remove(x, count);

	text[y] = l;

	/* inform the views */
	for ( YZView *v = view_list.first();v;v=view_list.next() ) {
			session->postEvent(YZEvent::mkEventInvalidateLine( v->myId,y ));
	}
#if 0
	for ( QMap<int,YZView*>::iterator it=view_list.begin(); it!=view_list.end(); it++ ) {
			YZView *v = it.data();
			session->postEvent(YZEvent::mkEventInvalidateLine( v->myId, y ));
	}
#endif
}

void YZBuffer::addNewLine( int col, int line ) {
	QString l=findLine(line);
	if (l.isNull()) return;

	//replace old line
	text[ line ] = l.left( col );

	//add new line
	QString newline = l.mid( col );
	if ( newline.isNull() ) newline = QString( "" );
	QStringList::Iterator it = text.at( line );
	text.insert( ++it, newline );
	/* inform the views */
	updateAllViews();
}

void YZBuffer::deleteLine( int line ) {
	if ( text.count() > 1 )
	 text.erase( text.at( line ) );
	else
		text[ line ] = "";
	updateAllViews(); //hmm ...
}

void YZBuffer::addView (YZView *v) {
	yzDebug() << "BUFFER: addView" << endl;
//	view_list.insert(v->myId, v );
	view_list.append( v );
	v->redrawScreen();
}

void YZBuffer::updateAllViews() {
	/*QValueList<YZView*>::iterator it;
	for ( it = view_list.begin(); it != view_list.end(); ++it ) {
		( *it )->redrawScreen();
	}*/
	for ( YZView *v = view_list.first();v;v=view_list.next() ) v->redrawScreen();
#if 0
	for ( QMap<int,YZView*>::iterator it=view_list.begin(); it!=view_list.end(); it++ ) {
		it.data()->redrawScreen();
	}
#endif
	
}

void  YZBuffer::addLine(QString &l) {
	text.append(l);
}

QString	YZBuffer::findLine(unsigned int line) {
	//we need to check this line exists.
	//the guy i talked with on IRC was right to doubt about it :)
	//so I return QString::null then for each call we need to check for if (!line.isNull())
	//trying if(!line) is NOT working (read QString doc)
	if ( text.count() <= line) return QString::null;
	else return text[ line ];
}

void YZBuffer::load() {
	text.clear();
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

YZView* YZBuffer::findView( int uid ) {
	for ( YZView *v = view_list.first();v;v=view_list.next() ) {
		if ( v->myId == uid ) return v;
	}
	return NULL;
	//return ( YZView* )view_list[ uid ];
}

//motion calculations

yz_point YZBuffer::motionPosition( int /*xstart*/, int /*ystart*/, YZMotion /*regexp*/ ) {

}

