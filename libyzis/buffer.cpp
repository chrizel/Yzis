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
	myId = YZSession::mNbBuffers++;
	mSession = sess;
	if ( !_path.isNull() )
		mPath = _path;
//	else mPath = "/tmp/yzisnew" + myId;

	load();
	mSession->addBuffer( this );
}

YZBuffer::~YZBuffer() {
	mText.clear();
}

void YZBuffer::addChar (unsigned int x, unsigned int y, const QString& c) {
	/* brute force, we'll have events specific for that later on */
	QString l=findLine(y);
	if (l.isNull()) return;

	l.insert(x, c);

	mText[y] = l;

	/* inform the views */
	QValueList<YZView*>::iterator it;
	for ( it = mViews.begin(); it != mViews.end(); ++it ) {
		YZView *v = *it;
		mSession->postEvent( YZEvent::mkEventInvalidateLine( v->myId,y ) );
	}

}

void YZBuffer::chgChar (unsigned int x, unsigned int y, const QString& c) {
	/* brute force, we'll have events specific for that later on */
	QString l=findLine(y);
	if (l.isNull()) return;

	/* do the actual modification */
	l.remove(x, 1);
	l.insert(x, c);

	mText[y] = l;

	/* inform the views */
	QValueList<YZView*>::iterator it;
	for ( it = mViews.begin(); it != mViews.end(); ++it ) {
		YZView *v = *it;
		mSession->postEvent( YZEvent::mkEventInvalidateLine( v->myId,y ) );
	}
}

void YZBuffer::delChar (unsigned int x, unsigned int y, unsigned int count) {
	/* brute force, we'll have events specific for that later on */
	QString l=findLine(y);
	if (l.isNull()) return;

	/* do the actual modification */
	l.remove(x, count);

	mText[y] = l;

	/* inform the views */
	QValueList<YZView*>::iterator it;
	for ( it = mViews.begin(); it != mViews.end(); ++it ) {
		YZView *v = *it;
		mSession->postEvent( YZEvent::mkEventInvalidateLine( v->myId,y ) );
	}
}

void YZBuffer::addNewLine( unsigned int col, unsigned int line ) {
	yzDebug() << "NB lines in buffer: " << mText.count() << " adding line at : " << line << endl;
	if ( line == getLines() ) {//we are adding a line, fake being at end of last line
		line --;
		col = mText[ line ].length(); 
	}

	QString l=findLine(line);
	if (l.isNull()) return;

	//replace old line
	mText[ line ] = l.left( col );

	//add new line
	QString newline = l.mid( col );
	if ( newline.isNull() ) newline = QString( "" );
	QStringList::Iterator it = mText.at( line );
	mText.insert( ++it, newline );
	/* inform the views */
	updateAllViews();
}

void YZBuffer::deleteLine( unsigned int line ) {
	if ( mText.count() > 1 )
	 mText.erase( mText.at( line ) );
	else
		mText[ line ] = "";
	updateAllViews(); //hmm ...
}

void YZBuffer::addView (YZView *v) {
//	view_list.insert(v->myId, v );
	QValueList<YZView*>::iterator it;
	for ( it = mViews.begin(); it != mViews.end(); ++it ) {
		YZView *vi = ( *it );
		if ( vi == v ) return; // don't append twice
	}
	yzDebug() << "BUFFER: addView" << endl;
	mViews.append( v );
	v->redrawScreen();
}

void YZBuffer::updateAllViews() {
	QValueList<YZView*>::iterator it;
	for ( it = mViews.begin(); it != mViews.end(); ++it ) {
		YZView *v = *it;
		v->redrawScreen();
	}
}

void  YZBuffer::addLine(const QString &l) {
	yzDebug() << "Adding new line : " << l << "$" << endl;
	mText.append(l);
}

QString	YZBuffer::findLine(unsigned int line) {
	//we need to check this line exists.
	//the guy i talked with on IRC was right to doubt about it :)
	//so I return QString::null then for each call we need to check for if (!line.isNull())
	//trying if(!line) is NOT working (read QString doc)
	if ( mText.count() <= line) return QString::null;
	else return mText[ line ];
}

void YZBuffer::load() {
	mText.clear();
	QFile file( mPath );
	//opens and eventually create the file
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.atEnd() ) {
			QString line(stream.readLine() ); // line of text excluding '\n'
			addLine( line );
		}
		file.close();
	}
	if ( ! mText.count() ) addLine(" ");
	updateAllViews();
}

void YZBuffer::save() {
	if (mPath.isEmpty())
		return;
	QFile file( mPath );
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		for ( QStringList::Iterator it = mText.begin(); it != mText.end(); ++it )
			stream << *it << "\n";
		file.close();
	}
}


YZView* YZBuffer::findView( unsigned int uid ) {
	yzDebug() << "Buffer: findView " << uid << endl;
	QValueList<YZView*>::iterator it;
	for ( it = mViews.begin(); it != mViews.end(); ++it ){
		YZView* v = ( *it );
		if ( v->myId == uid ) {
			yzDebug() << "Buffer: View " << uid << " found" << endl;
			return v;
		}
	}
	return NULL;
}

//motion calculations

yz_point YZBuffer::motionPosition( unsigned int /*xstart*/, unsigned int /*ystart*/, YZMotion /*regexp*/ ) {
	yz_point e;
	return e;
}

YZView* YZBuffer::firstView() {
	if (  mViews.first() != NULL ) 
		return mViews.first();
	else yzDebug() << "No VIEW !!!" << endl;
	return NULL;//crash me :)
}

