/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
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

/**
  YZAction: 
  	Here are all buffer modifications methods. 
	If you need to add a buffer modification method, you are at the _RIGHT_ place.
*/
			
   

#include "view.h"
#include "action.h"
#include "debug.h"
#include "registers.h"
#include "cursor.h"
#include "internal_options.h"
#include "buffer.h"
#include "session.h"

#include <stdlib.h>
#include <assert.h>
#include <qclipboard.h>
#include <qpaintdevice.h>

YZAction::YZAction( YZBuffer* buffer ) {
	mBuffer = buffer;
}
YZAction::~YZAction( ) {
}

#define CONFIGURE_VIEWS \
	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() ) \
		it->setPaintAutoCommit( false )

#define COMMIT_VIEWS_CHANGES \
	for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() ) \
		it->commitPaintEvent();

void YZAction::insertChar( YZView* pView, const YZCursor& pos, const QString& text ) {
	CONFIGURE_VIEWS;
	if( pos.getY() >= mBuffer->lineCount() )
		mBuffer->insertNewLine( pos.getX(), pos.getY() );
	mBuffer->insertChar( pos.getX(), pos.getY(), text );
	pView->moveXY( pos.getX() + text.length(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::replaceText( YZView* pView, const YZCursor& pos, unsigned int replacedLength, const QString& text ) {
	CONFIGURE_VIEWS;
	mBuffer->delChar( pos.getX(), pos.getY(), replacedLength );
	mBuffer->insertChar( pos.getX(), pos.getY(), text );
	pView->moveXY( pos.getX() + text.length(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::replaceChar( YZView* pView, const YZCursor& pos, const QString& text ) {
	CONFIGURE_VIEWS;
	mBuffer->delChar( pos.getX(), pos.getY(), text.length() );
	mBuffer->insertChar( pos.getX(), pos.getY(), text );
	pView->moveXY( pos.getX() + text.length(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::deleteChar( YZView* pView, const YZCursor& pos, unsigned int len ) {
	CONFIGURE_VIEWS;
	mBuffer->delChar( pos.getX(), pos.getY(), len );
	pView->moveXY( pos.getX(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::appendLine( YZView* pView, const QString& text ) {
	CONFIGURE_VIEWS;
	unsigned int y = mBuffer->lineCount();
	mBuffer->insertNewLine( 0, y );
	mBuffer->insertChar( 0, y, text );
	pView->moveXY( text.length(), y );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::insertNewLine( YZView* pView, const YZCursor& pos ) {
	CONFIGURE_VIEWS;
	mBuffer->insertNewLine( pos.getX(), pos.getY() );
	pView->moveXY( 0, pos.getY() + 1 );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::replaceLine( YZView* pView, const YZCursor& pos, const QString &text ) {
	CONFIGURE_VIEWS;
	mBuffer->replaceLine( text, pos.getY() );
	pView->moveXY( text.length(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::insertLine( YZView* pView, const YZCursor& pos, const QString &text ) {
	CONFIGURE_VIEWS;
	mBuffer->insertLine( text, pos.getY() );
	pView->moveXY( text.length(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::deleteLine( YZView* pView, const YZCursor& pos, unsigned int len, const QValueList<QChar> &reg ) {
	CONFIGURE_VIEWS;
	copyLine(pView, pos, len, reg);
	for ( unsigned int i = 0; i < len && pos.getY() < mBuffer->lineCount(); i++ )
		mBuffer->deleteLine( pos.getY() );
	pView->moveXY( 0, pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::copyLine( YZView* , const YZCursor& pos, unsigned int len, const QValueList<QChar> &reg ) {
	YZCursor mPos( pos );

	unsigned int bY = mPos.getY();
	QStringList buff;
	QString text = "";
	QString line;
	buff << QString::null;
	for ( unsigned int i = 0; i < len && mPos.getY() < mBuffer->lineCount(); i++ ) {
		line = mBuffer->textline( bY + i );
		buff << line;
		text += line + "\n";
	}
	buff << QString::null;
	if ( QPaintDevice::x11AppDisplay() ) {
		QApplication::clipboard()->setText( text, QClipboard::Clipboard );
	}

	for ( QValueList<QChar>::const_iterator it = reg.begin(); it != reg.end( ); it++ )
		YZSession::mRegisters.setRegister( *it, buff );
}


//copyArea and deleteArea have very similar code, if you modify one, you probably need to check the other
void YZAction::copyArea( YZView* pView, const YZCursor& beginCursor, const YZCursor& endCursor, const QValueList<QChar> &reg ) {
	yzDebug() << "Copying from X " << beginCursor.getX() << " to X " << endCursor.getX() << endl;

	YZCursor begin(beginCursor <= endCursor ? beginCursor : endCursor),
		end(beginCursor <= endCursor ? endCursor : beginCursor);
	YZBuffer *yzb=pView->myBuffer();
	YZCursor top(0, 0, 0), bottom(0, yzb->textline(yzb->lineCount()-1).length(), yzb->lineCount()-1);
	assert(begin >= top && end <= bottom);

	QStringList buff;
	unsigned int bX = begin.getX();
	unsigned int bY = begin.getY();
	unsigned int eX = end.getX();
	unsigned int eY = end.getY();

	if ( eY >= mBuffer->lineCount() ) return; //something's wrong => abort

	if((pView->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL ||
	   pView->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL_LINE)
			&& yzb->textline(eY).length() > 0)
		eX++;

	YZCursor mPos( begin );

	yzDebug() << "Cursors : " << bX << ","<< bY << " " << eX << "," << eY << endl;

	bool lineCopied = bY != eY;

	/* 1. copy the part of the current line */
	QString b = mBuffer->textline( bY );
	yzDebug() << "Current Line " << b << endl;
	if ( !lineCopied )
		buff << b.mid( bX, eX - bX );
	else
		buff << b.mid( bX );

	/* 2. copy whole lines */
	unsigned int curY = bY + 1;

	while ( eY > curY )
		buff << mBuffer->textline( curY++ );

	/* 3. copy the part of the last line */
	if ( lineCopied ) {
		b = mBuffer->textline( curY );
		buff << b.left( eX );
	}

	QString text = buff.join( "\n" );
	if ( QPaintDevice::x11AppDisplay() ) {
		QApplication::clipboard()->setText( text, QClipboard::Clipboard );
	}
	yzDebug() << "Copied " << buff << endl;
	for ( QValueList<QChar>::const_iterator it = reg.begin(); it != reg.end( ); it++ )
		YZSession::mRegisters.setRegister( *it, buff );


}

//copyArea and deleteArea have very similar code, if you modify one, you probably need to check the other
void YZAction::deleteArea( YZView* pView, const YZCursor& beginCursor, const YZCursor& endCursor, const QValueList<QChar> &reg ) {
	CONFIGURE_VIEWS;
	yzDebug() << "Deleting from X " << beginCursor.getX() << " to X " << endCursor.getX() << endl;
	QStringList buff;

	YZCursor begin(beginCursor <= endCursor ? beginCursor : endCursor),
		end(beginCursor <= endCursor ? endCursor : beginCursor);
	YZBuffer *yzb=pView->myBuffer();
	YZCursor top(0, 0, 0), bottom(0, yzb->textline(yzb->lineCount()-1).length(), yzb->lineCount()-1);
	assert(begin >= top && end <= bottom);

	unsigned int bX = begin.getX();
	unsigned int bY = begin.getY();
	unsigned int eX = end.getX();
	unsigned int eY = end.getY();

	if ( eY >= mBuffer->lineCount() ) return; //something's wrong => abort

	if((pView->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL ||
	   pView->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL_LINE)
			&& yzb->textline(eY).length() > 0)
		eX++;

	YZCursor mPos( begin );

	yzDebug() << "Cursors : " << bX << ","<< bY << " " << eX << "," << eY << endl;

	bool lineDeleted = bY != eY;

	/* 1. delete the part of the current line */
	QString b = mBuffer->textline( bY );
	yzDebug() << "Current Line " << b << endl;
	if ( !lineDeleted ) {
		buff << b.mid( bX, eX - bX );
		yzDebug() << "Deleting 1 " << buff << endl;
		QString b2 = b.left( bX ) + b.mid( eX );
		yzDebug() << "New line is " << b2 << endl;
		mBuffer->replaceLine( b2 , bY );
	} else {
		buff << b.mid( bX );
		QString b2 = b.left( bX );
		mBuffer->replaceLine( b2 , bY );
	}

	/* 2. delete whole lines */
	unsigned int curY = bY + 1;

	yzDebug() << "End " << end.getY() << " " << curY << endl;
	while ( eY > curY ) {
		buff << mBuffer->textline( curY );
		mBuffer->deleteLine( curY );
		eY--;
	}

	if(lineDeleted) {
		b = mBuffer->textline( curY );
		buff << b.left( eX );
		mBuffer->replaceLine( b.mid( eX ), curY );
		if ( curY > 0 ) mergeNextLine( pView, curY - 1 );
	}
	yzDebug() << "Deleted " << buff << endl;
	for ( QValueList<QChar>::const_iterator it = reg.begin(); it != reg.end( ); it++ )
		YZSession::mRegisters.setRegister( *it, buff );

	pView->moveXY( beginCursor.getX(), beginCursor.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::mergeNextLine( YZView* pView, unsigned int y ) {
	CONFIGURE_VIEWS;
	QString line = mBuffer->textline( y );
	QString line2 = mBuffer->textline( y + 1 );
	line2.replace(QRegExp("^\\s*"),"");
	mBuffer->replaceLine( line + line2, y );
	mBuffer->deleteLine( y + 1 );
	pView->moveXY( line.length(), y );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::insertChar( YZView* pView, unsigned int X, unsigned int Y, const QString& text ) {
	YZCursor pos( pView, X, Y );
	insertChar( pView, pos, text );
}
void YZAction::replaceChar( YZView* pView, unsigned int X, unsigned int Y, const QString& text ) {
	YZCursor pos( pView, X, Y );
	replaceChar( pView, pos, text );
}
void YZAction::deleteChar( YZView* pView, unsigned int X, unsigned int Y, unsigned int len ) {
	YZCursor pos( pView, X, Y );
	deleteChar( pView, pos, len );
}
void YZAction::insertLine( YZView* pView, unsigned int Y, const QString &text ) {
	YZCursor pos( pView, 0, Y );
	insertLine( pView, pos, text );
}
void YZAction::insertNewLine( YZView* pView, unsigned int X, unsigned int Y ) {
	YZCursor pos( pView, X, Y );
	insertNewLine( pView, pos );
}
void YZAction::deleteLine( YZView* pView, unsigned int Y, unsigned int len, const QValueList<QChar>& regs ) {
	YZCursor pos( pView, 0, Y );
	deleteLine( pView, pos, len, regs );
}
void YZAction::replaceLine( YZView* pView, unsigned int Y, const QString& text ) {
	YZCursor pos( pView, 0, Y );
	replaceLine( pView, pos, text );
}

YZCursor YZAction::match( YZView* pView, YZCursor& mCursor, bool *found ) {
	QString matchers = pView->myBuffer()->getLocalStringOption("matchpairs");

	QString current = pView->myBuffer()->textline( mCursor.getY() );
	QChar cchar = current[ mCursor.getX() ];

	int i = 0;
	unsigned int j = 0;
	unsigned int curY = mCursor.getY();
	int count = 1;
	bool back = false;
	unsigned int start = 0;

	for ( i = 0; i < ( int )matchers.length() ; i++ ) {
		if ( matchers[ i ] == cchar ) {
			back = ( abs( i/2 ) * 2 ) != i;
			QChar c = matchers[ back ? i - 1 : i + 1 ]; //the character to match
			//now do the real search
			while ( curY < pView->myBuffer()->lineCount() && count > 0 ) {
				current = pView->myBuffer()->textline( curY );
				if ( back && mCursor.getY() == curY ) {
					if ( mCursor.getX() == 0) {
						curY--;
						current = pView->myBuffer()->textline( curY );
						start = current.length() - 1;
					} else
						start = mCursor.getX()-1;
				} else if ( !back && mCursor.getY() == curY )
					start =  mCursor.getX()+1;
				else
					start = back ? current.length() -1 : 0 ;

				for ( j = start; ( j < current.length() ) && ( count > 0 ) ; back ? j-- : j++ ) { //parse current line
					if ( current[ j ] == cchar ) {
						count++;
					} else if ( current[ j ] == c ) {
						count--; // we need to skip one more
					}
				}
				if ( count > 0 ) { //let's do another loop
					//current = pView->myBuffer()->textline( back ? --curY : ++curY );
					if ( back ) --curY;
					else curY++;
				}
			}
		}
	}
	if ( count == 0 ) {//found it !
		*found = true;
		yzDebug() << "Result action : " << ( back ? j+1 : j-1 ) << ", " << curY << endl;
		return YZCursor( pView, ( back ? j + 1 : j - 1 ), curY );
	}
	*found=false;
	return YZCursor( pView, 0,0 );
}

//mBegin is always the beginning of the search so if reverseSearch is true , we have mEnd < mBegin ;)
YZCursor YZAction::search( YZView* pView, const QString& what, const YZCursor& mBegin, const YZCursor& mEnd, bool reverseSearch, unsigned int *matchlength, bool *found ) {
	QRegExp ex( what );
	int currentMatchLine = mBegin.getY(); //start from current line
	//if you understand this line you are semi-god :)
	//if you don't understand, remove the IFs and see what it does when you have the cursor on the last/first column and start a search/reverse search
	unsigned int mMaxX = pView->myBuffer()->textline( currentMatchLine ).length() - 1;
	int currentMatchColumn = reverseSearch ? ( mBegin.getX() ? mBegin.getX() : 1 ) - 1 : \
		( mBegin.getX() < mMaxX ) ? mBegin.getX() + 1 : mBegin.getX(); //start from current column +/- 1

	//get current line
	QString l;

	for ( unsigned int i = currentMatchLine; i < pView->myBuffer()->lineCount() 
		&& ( (reverseSearch && i >= mEnd.getY()) || (!reverseSearch && i <= mEnd.getY()) ); 
			reverseSearch ? i-- : i++ ) {
			
		l = pView->myBuffer()->textline( i );
		yzDebug() << "Searching " << what << " in line : " << l << endl;
		int idx;
		if ( reverseSearch )
			idx = ex.searchRev( l, currentMatchColumn );
		else
			idx = ex.search( l, currentMatchColumn );

		if ( idx >= 0 ) {
			//check we are not past the mEnd
			if ( i == mEnd.getY() && ((!reverseSearch && (unsigned int)idx > mEnd.getX()) || (reverseSearch && (unsigned int)idx < mEnd.getX()))) 
				continue; //should exit the loop at next iteration anyway and return false

			//i really found it ? or is it a previous "found" ?
			if ( mBegin.getX() == ( unsigned int ) idx && reverseSearch && i == (unsigned int)currentMatchLine ) { //ok we did not move guy (col 0 or last col maybe ...)
				yzDebug() << "Only a fake match on this line, skip it" << endl;
				if ( reverseSearch )
					currentMatchColumn=-1;
				else
					currentMatchColumn=0; //reset the column (only valid for the first line we check)
				continue; //exit the IF
			}
			//really found it !
			currentMatchColumn = idx;
			currentMatchLine = i;
			*found = true;
			*matchlength = ex.matchedLength();
			return YZCursor(pView,currentMatchColumn, currentMatchLine);
		} else {
			yzDebug() << "No match on this line" << endl;
			if ( reverseSearch )
				currentMatchColumn=-1;
			else
				currentMatchColumn=0; //reset the column (only valid for the first line we check)
		}
	}
	*found = false;
	return YZCursor(pView,0,0);//fake result
}

