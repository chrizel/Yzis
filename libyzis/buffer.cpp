/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#include <assert.h>
#include <cstdlib>
#include <qfile.h>
#include <qtextstream.h>

#include "events.h"
#include "buffer.h"
#include "line.h"
#include "view.h"
#include "yzis.h"
#include "debug.h"

YZBuffer::YZBuffer(YZSession *sess, const QString& _path) {
	myId = YZSession::mNbBuffers++;
	mSession = sess;
	if ( !_path.isNull() )
		mPath = _path;
	else mPath = QString("/tmp/yzisnew%1").arg(myId); //we need this so that the buffer has a name at creation time (it will change when we load() a new file

	load();
	mSession->addBuffer( this );
}

YZBuffer::~YZBuffer() {
	mText.clear();
}

void YZBuffer::addChar (unsigned int x, unsigned int y, const QString& c) {
	/* brute force, we'll have events specific for that later on */
	QString l=data(y);
	if (l.isNull()) return;

	l.insert(x, c);

	at(y)->setData(l);

	/* inform the views */
	YZView *it;
	for ( it = mViews.first(); it != mViews.next(); ++it ) {
//		YZView *v = *it;
		mSession->postEvent( YZEvent::mkEventInvalidateLine( it->myId,y ) );
	}

}

void YZBuffer::chgChar (unsigned int x, unsigned int y, const QString& c) {
	/* brute force, we'll have events specific for that later on */
	QString l=data(y);
	if (l.isNull()) return;

	/* do the actual modification */
	l.remove(x, 1);
	l.insert(x, c);

	at(y)->setData(l);

	/* inform the views */
//	QValueList<YZView*>::iterator it;
	YZView *it;
	for ( it = mViews.first(); it; it=mViews.next() ) {
	//	YZView *v = *it;
		mSession->postEvent( YZEvent::mkEventInvalidateLine( it->myId,y ) );
	}
}

void YZBuffer::delChar (unsigned int x, unsigned int y, unsigned int count)
{
	yzDebug() << "YZBuffer::delChar(): at " << x << "," << y << ": " << count << endl;
	/* brute force, we'll have events specific for that later on */
	QString l=data(y);
	if (l.isNull()) return;

	/* do the actual modification */
	l.remove(x, count);

	at(y)->setData(l);

	/* inform the views */
	//QValueList<YZView*>::iterator it;
	YZView *it;
	for ( it = mViews.first(); it; it = mViews.next() ) {
//		YZView *v = *it;
		mSession->postEvent( YZEvent::mkEventInvalidateLine( it->myId,y ) );
	}
}

void YZBuffer::addNewLine( unsigned int col, unsigned int line ) {
	yzDebug() << "NB lines in buffer: " << mText.count() << " adding line at : " << line << endl;
	if ( line == lineCount() ) {//we are adding a line, fake being at end of last line
		line --;
		col = data(line).length(); 
	}

	QString l=data(line);
	if (l.isNull()) return;

	//replace old line
	at(line)->setData(l.left( col ));

	//add new line
	QString newline = l.mid( col );
	if ( newline.isNull() ) newline = QString( "" );
	mText.insert( line + 1, new YZLine(newline));
	/* inform the views */
	updateAllViews();
}

void YZBuffer::deleteLine( unsigned int line ) {
	if ( mText.count() > 1 )
	 mText.remove(line);
	else
		at(line)->setData("");
	updateAllViews(); //hmm ...
}

void YZBuffer::addView (YZView *v) {
//	QValueList<YZView*>::iterator it;
	YZView *it;
	for ( it = mViews.first(); it; it=mViews.next() ) {
//		YZView *vi = ( *it );
		if ( it == v ) return; // don't append twice
	}
	yzDebug() << "BUFFER: addView" << endl;
	mViews.append( v );
	v->redrawScreen();
	mSession->setCurrentView( v );
}

void YZBuffer::updateAllViews() {
//	QValueList<YZView*>::iterator it;
	YZView *it;
	for ( it = mViews.first(); it; it = mViews.next() ) {
//		YZView *v = *it;
		it->redrawScreen();
	}
}

void  YZBuffer::addLine(const QString &l) {
	mText.append(new YZLine(l));
}

QString	YZBuffer::data(unsigned int no)
{
#if 0
	//we need to check this line exists.
	//the guy i talked with on IRC was right to doubt about it :)
	//so I return QString::null then for each call we need to check for if (!line.isNull())
	//trying if(!line) is NOT working (read QString doc)
#endif

	YZLine *l = at(no);
	if(!l)
		return QString::null;
	else
		return l->data();
}

void YZBuffer::load(const QString& file) {
	mText.clear();
	if ( !file.isNull() ) { 
		//hmm changing file :), update Session !!!!
		mSession->updateBufferRecord( mPath, file, this );
		mPath = file;
	}
	QFile fl( mPath );
	//opens and eventually create the file
	if ( fl.open( IO_ReadOnly ) ) {
		QTextStream stream( &fl );
		while ( !stream.atEnd() ) {
			QString line(stream.readLine() ); // line of text excluding '\n'
			addLine( line );
		}
		fl.close();
	}
	if ( ! mText.count() ) addLine("");
	updateAllViews();
}

void YZBuffer::save() {
	if (mPath.isEmpty())
		return;
	QFile file( mPath );
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		for(YZLine *it = mText.first(); it; it = mText.next())
			stream << it->data() << "\n";
		file.close();
	}
}


YZView* YZBuffer::findView( unsigned int uid ) {
	yzDebug() << "Buffer: findView " << uid << endl;
//	QValueList<YZView*>::iterator it;
	YZView *it;
	for ( it = mViews.first(); it; it=mViews.next() ){
//		YZView* v = ( *it );
		yzDebug() << "Checking view " << uid << " for buffer " << fileName() << endl;
		if ( it->myId == uid ) {
			yzDebug() << "Buffer: View " << uid << " found" << endl;
			return it;
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

void YZBuffer::rmView(YZView *v) {
	int f = mViews.remove(v);
	yzDebug() << "buffer: removeView found " << f << " views" << endl;
}
