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
#include <qfileinfo.h>

#include "buffer.h"
#include "line.h"
#include "view.h"
#include "yzis.h"
#include "undo.h"
#include "debug.h"

YZBuffer::YZBuffer(YZSession *sess, const QString& _path) 
{
	myId = YZSession::mNbBuffers++;
	mSession = sess;
	if ( !_path.isNull() ) {
		mPath = _path;
	} else {
		// buffer at creation time should use a non existing temp filename
		// find a tmp file that does not exist
		do {
			mPath = QString("/tmp/yzisnew%1").arg(random());
		} while ( QFileInfo( mPath ).exists() == true );
		// there is still a possible race condition here...
	}
	mUndoBuffer = new YZUndoBuffer( this );
	load();
	mSession->addBuffer( this );
}

YZBuffer::~YZBuffer() {
	mText.clear();
	delete mUndoBuffer;
	// delete the temporary file if we haven't changed the file
}

// ------------------------------------------------------------------------
//                            Char Operations 
// ------------------------------------------------------------------------

void YZBuffer::insertChar(unsigned int x, unsigned int y, const QString& c) {
	YZASSERT_MSG( c.contains('\n')==false, QString("YZBuffer::insertChar( %1, %2, %3 ) - adding a char that contains a \\n").arg(x).arg(y).arg(c) )
	YZASSERT_MSG(y < lineCount(), QString("YZBuffer::insertChar( %1, %2, %3 ) but line %4 does not exist, buffer has %5 lines").arg( x ).arg( y ).arg( c ).arg( y ).arg( lineCount() ) );

	/* brute force, we'll have events specific for that later on */
	QString l=data(y);
	if (l.isNull()) return;

	YZASSERT_MSG(x <= l.length(), QString("YZBuffer::insertChar( %1, %2, %3 ) but col %4 does not exist, line has %5 columns").arg( x ).arg( y ).arg( c ).arg( x ).arg( l.length() ) );

	if (x > l.length()) {
		// if we let Qt proceed, it would append spaces to extend the line
		// and we do not want that
		return;
	}

	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, c, x, y );

	l.insert(x, c);
	at(y)->setData(l);

	/* inform the views */
	YZView *it;
	for ( it = mViews.first(); it ; it = mViews.next() )
		it->invalidateLine( y );

}

void YZBuffer::chgChar (unsigned int x, unsigned int y, const QString& c) {
	YZASSERT_MSG( c.contains('\n')==false, QString("YZBuffer::chgChar( %1, %2, %3 ) - adding a char that contains a \\n").arg(x).arg(y).arg(c) )
	YZASSERT_MSG( y < lineCount(), QString("YZBuffer::chgChar( %1, %2, %3 ) but line %4 does not exist, buffer has %5 lines").arg( x ).arg( y ).arg( c ).arg( y ).arg( lineCount() ) );

	/* brute force, we'll have events specific for that later on */
	QString l=data(y);
	if (l.isNull()) return;

	YZASSERT_MSG( x < l.length(), QString("YZBuffer::chgChar( %1, %2, %3 ) but col %4 does not exist, line has %5 columns").arg( x ).arg( y ).arg( c ).arg( x ).arg( l.length() ) );

	if (x >= l.length()) {
		// if we let Qt proceed, it will append spaces to extend the line
		return;
	}

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, 
	                                 l.mid(x,1), x, y );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, c, x, y );
	
	/* do the actual modification */
	l.remove(x, 1);
	l.insert(x, c);

	at(y)->setData(l);


	/* inform the views */
	YZView *it;
	for ( it = mViews.first(); it; it=mViews.next() ) {
		it->invalidateLine( y );
	}
}

void YZBuffer::delChar (unsigned int x, unsigned int y, unsigned int count)
{
	yzDebug() << "YZBuffer::delChar(): at " << x << "," << y 
				<< ": " << count << endl;
	YZASSERT_MSG( y < lineCount(), QString(
		"YZBuffer::delChar( %1, %2, %3 ) but line %4 does not exist"
		", buffer has %5 lines").arg( x ).arg( y ).arg( count )
		.arg( y ).arg( lineCount() ) );

	/* brute force, we'll have events specific for that later on */
	QString l=data(y);
	if (l.isNull()) return;

	YZASSERT_MSG( x < l.length(), QString(
		"YZBuffer::delChar( %1, %2, %3 ) but col %4 does not exist,"
		" line has %5 columns").arg( x ).arg( y ).arg( count )
		.arg( x ).arg( l.length() ) );

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, 
	                                   l.mid(x,count), x, y );
	
	/* do the actual modification */
	l.remove(x, count);

	at(y)->setData(l);

	/* inform the views */
	YZView *it;
	for ( it = mViews.first(); it; it = mViews.next() ) {
		it->invalidateLine( y );
	}
}

// ------------------------------------------------------------------------
//                            Line Operations 
// ------------------------------------------------------------------------

void  YZBuffer::appendLine(const QString &l) {
	YZASSERT_MSG( l.contains('\n')==false, "YZBuffer::appendLine() : adding a line with '\n' inside" );

	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, 
	                                 QString(), 0, lineCount() );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, 
	                                   l,  0, lineCount());

	mText.append(new YZLine(l));
	updateAllViews();
}


void  YZBuffer::insertLine(const QString &l, unsigned int line) {
	YZASSERT_MSG( l.contains('\n')==false, "YZBuffer::insertLine() : adding a line with '\n' inside" );
	YZASSERT_MSG( line < lineCount(), QString("YZBuffer::insertLine( %1 ) but line does not exist, buffer has %3 lines").arg( line ).arg( lineCount() ) );

	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, 
	                                   QString(), 0, line );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, 
	                                   l, 0, line );

	mText.insert(line, new YZLine(l));
	updateAllViews();
}

void YZBuffer::insertNewLine( unsigned int col, unsigned int line ) {
	YZASSERT_MSG( line < lineCount(), QString(
		"YZBuffer::insertNewLine( %1, %2 ) but line %3 does not exist,"
		" buffer has %4 lines").arg( line ).arg( col ).arg( line )
		.arg( lineCount() ) );
	if ( line == lineCount() ) {//we are adding a line, fake being at end of last line
		line --;
		col = data(line).length(); 
	}

	QString l=data(line);
	if (l.isNull()) return;

	YZASSERT_MSG( col <= l.length(), QString(
		"YZBuffer::insertNewLine( %1, %2 ) but column %3 does not exist,"
		" line has %4 columns").arg( line ).arg( col ).arg( col )
		.arg( l.length() ) );
	if (col > l.length() ) return;

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, 
	                                   l.mid(col), col, line );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, 
	                                   "", col, line+1 );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, 
	                                   l.mid(col), 0, line+1 );

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
	YZASSERT_MSG( line < lineCount(), QString("YZBuffer::deleteLine( %1 ) but line does not exist, buffer has %3 lines").arg( line ).arg( lineCount() ) );

	if (line >= lineCount()) return;


	if (lineCount() > 1) {
		mUndoBuffer->addBufferOperation( YZBufferOperation::DELLINE, 
										 QString(), 0, line );
		mText.remove(line);
	} else {
		mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, 
										 data(0), 0, line );
		at(0)->setData("");
	}

	updateAllViews(); //hmm ...
}

void YZBuffer::replaceLine( const QString& l, unsigned int line ) {
	YZASSERT_MSG( l.contains('\n')==false, "YZBuffer::replaceLine() : replacing a line with '\n' inside" );
	YZASSERT_MSG( line < lineCount(), QString("YZBuffer::replaceLine( %1 ) but line does not exist, buffer has %3 lines").arg( line ).arg( lineCount() ) );
	
	if ( data( line ).isNull() ) return;

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, 
	                                   data(line), 0, line );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, 
	                                   l, 0, line );

	at(line)->setData(l);
	/* inform the views */
	YZView *it;
	for ( it = mViews.first(); it ; it = mViews.next() )
		it->invalidateLine( line );
}

// ------------------------------------------------------------------------
//                            Content Operations 
// ------------------------------------------------------------------------

void YZBuffer::clearText() const
{
	( (QPtrList<YZLine> )mText).clear();
}

QString	YZBuffer::data(unsigned int no) const
{
#if 0
	//we need to check this line exists.
	//the guy i talked with on IRC was right to doubt about it :)
	//so I return QString::null then for each call we need to check for if (!line.isNull())
	//trying if(!line) is NOT working (read QString doc)
#endif

	YZLine *l = ( ( QPtrList<YZLine> )mText ).at(no);
	if(!l)
		return QString::null;
	else
		return l->data();
}

QString YZBuffer::getWholeText() const {
	QString text;
	for ( int i = 0 ; i < lineCount() ; i++ )
		text += ( ( QPtrList<YZLine> )mText ).at( i )->data() + "\n";
//	for(YZLine *it = mText.first(); it; it = mText.next())
//		text += it->data() + "\n";
	text.truncate( text.length()-1 );
	return text;
}

uint YZBuffer::getWholeTextLength() const {
	uint length = 0;
	
	for ( uint i = 0 ; i < lineCount() ; i++ ) {
		/* Should this include \n? Does for now*/
		length += ( ( QPtrList<YZLine> )mText ).at( i )->data().length() + 1;
	}
	
	return length;
}

uint YZBuffer::firstNonBlankChar( uint line )
{
	uint i=0;
	QString s = data(line);
	if (s == QString::null) return 0;
	while( s[i].isSpace() && i < s.length()) {
		i++;
	}
	return i;
}
	
// ------------------------------------------------------------------------
//                            File Operations 
// ------------------------------------------------------------------------

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
			appendLine( line );
		}
		fl.close();
	}
	if ( ! mText.count() ) {
		mUndoBuffer->setInsideUndo( true );
		appendLine("");
		mUndoBuffer->setInsideUndo( false );
	}
	updateAllViews();
}

void YZBuffer::save() {
	if (mPath.isEmpty())
		return;
	QFile file( mPath );
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		for(YZLine *it = mText.first(); it; it = mText.next()) {
			if (it != mText.getFirst()) {
				stream << "\n";
			}
			stream << it->data();
		}
		file.close();
	}
}

// ------------------------------------------------------------------------
//                            View Operations 
// ------------------------------------------------------------------------

void YZBuffer::addView (YZView *v) {
	YZView *it;
	for ( it = mViews.first(); it; it=mViews.next() ) {
		if ( it == v ) return; // don't append twice
	}
	yzDebug() << "BUFFER: addView" << endl;
	mViews.append( v );
	mSession->setCurrentView( v );
}

YZView* YZBuffer::findView( unsigned int uid ) {
	yzDebug() << "Buffer: findView " << uid << endl;
	YZView *it;
	for ( it = mViews.first(); it; it=mViews.next() ){
		yzDebug() << "Checking view " << uid << " for buffer " << fileName() << endl;
		if ( it->myId == uid ) {
			yzDebug() << "Buffer: View " << uid << " found" << endl;
			return it;
		}
	}
	return NULL;
}

void YZBuffer::updateAllViews() {
	YZView *it;
	for ( it = mViews.first(); it; it = mViews.next() ) {
		it->redrawScreen();
	}
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

// ------------------------------------------------------------------------
//                            Undo/Redo Operations
// ------------------------------------------------------------------------

QString YZBuffer::undoLast( const QString& , YZCommandArgs args ) {
	yzDebug() << "buffer: undoLast" << endl;
	mUndoBuffer->undo();	
	
	//reset the input buffer of the originating view
	args.view->purgeInputBuffer();

	//return something
	return QString::null;
}
