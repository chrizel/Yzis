/**
 * $Id$
 */

#include "events.h"
#include <cstdlib>
#include <qfile.h>
#include <qtextstream.h>
#include "buffer.h"
#include "view.h"
#include "yzis.h"
#include "debug.h"
#include <assert.h>

YZBuffer::YZBuffer(YZSession *sess, const QString& _path) {
	myId = YZSession::nbBuffers++;
	session = sess;
	if ( !_path.isNull() )
		path = _path;
	else path = "new" + myId;

	if (!_path.isEmpty()) load();
	else {
/*		QString blah( "" );
		addLine(blah);*/
	}
	session->addBuffer( this );
}

YZBuffer::~YZBuffer() {
	text.clear();
}

void YZBuffer::addChar (int x, int y, const QString& c) {
	/* brute force, we'll have events specific for that later on */
	QString l=findLine(y);
	if (l.isNull()) return;

	l.insert(x, c);

	text[y] = l;

	/* inform the views */
	QValueList<YZView*>::iterator it;
	for ( it = view_list.begin(); it != view_list.end(); ++it ) {
		YZView *v = *it;
		session->postEvent( YZEvent::mkEventInvalidateLine( v->myId,y ) );
	}

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
	QValueList<YZView*>::iterator it;
	for ( it = view_list.begin(); it != view_list.end(); ++it ) {
		YZView *v = *it;
		session->postEvent( YZEvent::mkEventInvalidateLine( v->myId,y ) );
	}
}

void YZBuffer::delChar (int x, int y, int count) {
	/* brute force, we'll have events specific for that later on */
	QString l=findLine(y);
	if (l.isNull()) return;

	/* do the actual modification */
	l.remove(x, count);

	text[y] = l;

	/* inform the views */
	QValueList<YZView*>::iterator it;
	for ( it = view_list.begin(); it != view_list.end(); ++it ) {
		YZView *v = *it;
		session->postEvent( YZEvent::mkEventInvalidateLine( v->myId,y ) );
	}
}

void YZBuffer::addNewLine( int col, int line ) {
	yzDebug() << "NB lines in buffer: " << text.count() << " adding line at : " << line << endl;
	if ( line == getLines() ) {//we are adding a line, fake being at end of last line
		line --;
		col = text[ line ].length(); 
	}

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
//	view_list.insert(v->myId, v );
	QValueList<YZView*>::iterator it;
	for ( it = view_list.begin(); it != view_list.end(); ++it ) {
		YZView *vi = ( *it );
		if ( vi == v ) return; // don't append twice
	}
	yzDebug() << "BUFFER: addView" << endl;
	view_list.append( v );
	v->redrawScreen();
}

void YZBuffer::updateAllViews() {
	QValueList<YZView*>::iterator it;
	for ( it = view_list.begin(); it != view_list.end(); ++it ) {
		YZView *v = *it;
		v->redrawScreen();
	}
}

void  YZBuffer::addLine(const QString &l) {
	yzDebug() << "Adding new line : " << l << endl;
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
	yzDebug() << "Buffer: findView " << uid << endl;
	QValueList<YZView*>::iterator it;
	for ( it = view_list.begin(); it != view_list.end(); ++it ){
		YZView* v = ( *it );
		if ( v->myId == uid ) {
			yzDebug() << "Buffer: View " << uid << " found" << endl;
			return v;
		}
	}
	return NULL;
}

//motion calculations

yz_point YZBuffer::motionPosition( int /*xstart*/, int /*ystart*/, YZMotion /*regexp*/ ) {
	yz_point e;
	return e;
}

YZView* YZBuffer::firstView() {
	if (  view_list.first() != NULL ) 
		return view_list.first();
	else yzDebug() << "No VIEW !!!" << endl;
	return NULL;//crash me :)
}

