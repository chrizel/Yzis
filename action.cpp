/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>,
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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
#if QT_VERSION < 0x040000
#include <qclipboard.h>
#include <qpaintdevice.h>
#else
#include <QX11Info>
#include <QApplication>
#include <QClipboard>
#endif

YZAction::YZAction( YZBuffer* buffer ) {
	mBuffer = buffer;
}
YZAction::~YZAction( ) {
}

#if QT_VERSION < 0x040000
	#define CONFIGURE_VIEWS \
		{ for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() ) \
		it->setPaintAutoCommit( false ); } 

	#define COMMIT_VIEWS_CHANGES \
		{ for ( YZView* it = mBuffer->views().first(); it; it = mBuffer->views().next() ) \
		it->commitPaintEvent(); }
#else
	#define CONFIGURE_VIEWS \
		{ for ( int aabb = 0; aabb < mBuffer->views().size(); ++aabb ) \
		mBuffer->views().at(aabb)->setPaintAutoCommit( false ); } 

	#define COMMIT_VIEWS_CHANGES \
		{ for ( int aabb = 0; aabb < mBuffer->views().size(); ++aabb ) \
		mBuffer->views().at(aabb)->commitPaintEvent(); }
#endif

void YZAction::insertChar( YZView* pView, const YZCursor& pos, const QString& text ) {
	CONFIGURE_VIEWS;
	if( pos.getY() >= mBuffer->lineCount() )
		mBuffer->insertNewLine( pos.getX(), pos.getY() );
	mBuffer->insertChar( pos.getX(), pos.getY(), text );
	pView->gotoxyAndStick( pos.getX() + text.length(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::replaceText( YZView* pView, const YZCursor& pos, unsigned int replacedLength, const QString& text ) {
	yzDebug() << "replaceText :" << pos << " length : " << replacedLength << " text:" << text << endl;
	CONFIGURE_VIEWS;
	mBuffer->delChar( pos.getX(), pos.getY(), replacedLength );
	mBuffer->insertChar( pos.getX(), pos.getY(), text );
	pView->gotoxyAndStick( pos.getX() + text.length(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::replaceChar( YZView* pView, const YZCursor& pos, const QString& text ) {
	CONFIGURE_VIEWS;
	mBuffer->delChar( pos.getX(), pos.getY(), text.length() );
	mBuffer->insertChar( pos.getX(), pos.getY(), text );
	pView->gotoxyAndStick( pos.getX() + text.length(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::deleteChar( YZView* pView, const YZCursor& pos, unsigned int len ) {
	CONFIGURE_VIEWS;
	mBuffer->delChar( pos.getX(), pos.getY(), len );
	pView->gotoxyAndStick( pos.getX(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::appendLine( YZView* pView, const QString& text ) {
	CONFIGURE_VIEWS;
	unsigned int y = mBuffer->lineCount();
	mBuffer->insertNewLine( 0, y );
	mBuffer->insertChar( 0, y, text );
	pView->gotoxyAndStick( text.length(), y );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::insertNewLine( YZView* pView, const YZCursor& pos ) {
	CONFIGURE_VIEWS;
	mBuffer->insertNewLine( pos.getX(), pos.getY() );
	pView->gotoxyAndStick( 0, pos.getY() + 1 );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::replaceLine( YZView* pView, const YZCursor& pos, const QString &text ) {
	CONFIGURE_VIEWS;
	mBuffer->replaceLine( text, pos.getY() );
	pView->gotoxyAndStick( text.length(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::insertLine( YZView* pView, const YZCursor& pos, const QString &text ) {
	CONFIGURE_VIEWS;
	mBuffer->insertLine( text, pos.getY() );
	pView->gotoxyAndStick( text.length(), pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

#if QT_VERSION < 0x040000
void YZAction::deleteLine( YZView* pView, const YZCursor& pos, unsigned int len, const QValueList<QChar> &reg ) {
#else
void YZAction::deleteLine( YZView* pView, const YZCursor& pos, unsigned int len, const QList<QChar> &reg ) {
#endif
	CONFIGURE_VIEWS;
	copyLine(pView, pos, len, reg);
	for ( unsigned int i = 0; i < len && pos.getY() < mBuffer->lineCount(); i++ )
		mBuffer->deleteLine( pos.getY() );
	pView->gotoxyAndStick( 0, pos.getY() );
	COMMIT_VIEWS_CHANGES;
}

#if QT_VERSION < 0x040000
void YZAction::copyLine( YZView* , const YZCursor& pos, unsigned int len, const QValueList<QChar> &reg ) {
#else
void YZAction::copyLine( YZView* , const YZCursor& pos, unsigned int len, const QList<QChar> &reg ) {
#endif
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
#ifndef WIN32
#if QT_VERSION < 0x040000
	if ( QPaintDevice::x11AppDisplay() )
#else
	if ( QX11Info::display() )
#endif
#endif
		QApplication::clipboard()->setText( text, QClipboard::Clipboard );
	
#if QT_VERSION < 0x040000
	QValueList<QChar>::const_iterator it = reg.begin(), end = reg.end();
	for ( ; it != end; ++it )
		YZSession::mRegisters->setRegister( *it, buff );
#else
	for ( int ab = 0 ; ab < reg.size(); ++ab )
		YZSession::mRegisters->setRegister( reg.at(ab), buff );
#endif
}


#if QT_VERSION < 0x040000
void YZAction::copyArea( YZView* /*pView*/, const YZInterval& i, const QValueList<QChar> &reg ) {
#else
void YZAction::copyArea( YZView* pView, const YZInterval& i, const QList<QChar> &reg ) {
#endif
	QStringList buff;
	unsigned int bX = i.fromPos().getX();
	unsigned int bY = i.fromPos().getY();
	unsigned int eX = i.toPos().getX();
	unsigned int eY = i.toPos().getY();

	bool copyWholeLines = ( bX == 0 && i.from().closed() && eX == 0 && i.to().opened() );
	if ( copyWholeLines )
		buff << QString::null;

	if ( i.from().opened() ) ++bX;
	if ( i.to().closed() ) ++eX;

	QString l = mBuffer->textline( bY );
	if ( bY == eY ) {
		buff << l.left( eX ).mid( bX );
	} else {
		buff << l.mid( bX );
		for ( unsigned int y = bY+1; y < eY; y++ )
			buff << mBuffer->textline( y );
		buff << mBuffer->textline( eY ).left( eX );
	}

#ifndef WIN32
#if QT_VERSION < 0x040000
	if ( QPaintDevice::x11AppDisplay() )
#else
	if ( QX11Info::display() )
#endif
#endif
		QApplication::clipboard()->setText( mBuffer->getText( i ).join("\n"), QClipboard::Clipboard );
	
	yzDebug() << "Copied " << buff << endl;
#if QT_VERSION < 0x040000
	QValueList<QChar>::const_iterator it = reg.begin(), endd = reg.end();
	for ( ; it != endd; ++it )
		YZSession::mRegisters->setRegister( *it, buff );
#else
	for ( int ab = 0 ; ab < reg.size(); ++ab )
		YZSession::mRegisters->setRegister( reg.at(ab), buff );
#endif


}

void YZAction::replaceArea( YZView* /*pView*/, const YZInterval& i, const QStringList& text ) {
	CONFIGURE_VIEWS;
	QStringList t = text;

	unsigned int bX = i.fromPos().getX();
	unsigned int bY = i.fromPos().getY();
	unsigned int eX = i.toPos().getX();
	unsigned int eY = i.toPos().getY();

	if ( i.from().opened() ) ++bX;
	if ( i.to().opened() && eX > 0 ) --eX;
	if ( i.to().opened() && eX == 0 ) {
		--eY;
		eX = mBuffer->textline( eY ).length() - 1;
	}
	QString bL = mBuffer->textline( bY ).left( bX );
	QString eL = mBuffer->textline( eY ).mid( eX + 1 );

	unsigned int rH = text.size();
	unsigned int dH = eY - bY + 1;
	if ( rH == 0 ) {
		t << "";
		rH = 1;
	}

	if ( rH > 1 ) {
		mBuffer->replaceLine( bL + t[ 0 ], bY );
		if ( eY == bY ) {
			mBuffer->insertLine( t[ rH - 1 ] + eL, eY + 1 );
		} else {
			mBuffer->replaceLine( t[ rH - 1 ] + eL, eY );
			--dH;
		}
		--rH;
	} else {
		mBuffer->replaceLine( bL + t[ 0 ] + eL, bY );
		if ( eY != bY ) {
			mBuffer->deleteLine( eY );
			--dH;
		}
	}
	unsigned int j;
	unsigned int max = qMin( rH, dH );
	for ( j = 1; j < max; j++ ) {
		mBuffer->replaceLine( t[ j ], bY + j );
	}
	if ( j == rH ) { // remove lines
		for( ; j < dH; j++ ) {
			mBuffer->deleteLine( bY + rH );
		}
	} else { // insert lines
		for( ; j < rH; j++ ) {
			mBuffer->insertLine( t[ j ], bY + j );
		}
	}
	
	COMMIT_VIEWS_CHANGES;
}

#if QT_VERSION < 0x040000
void YZAction::deleteArea( YZView* pView, const YZInterval& i, const QValueList<QChar> &reg ) {
#else
void YZAction::deleteArea( YZView* pView, const YZInterval& i, const QList<QChar> &reg ) {
#endif
	CONFIGURE_VIEWS;

	QStringList buff = mBuffer->getText( i );

	unsigned int bX = i.fromPos().getX();
	unsigned int bY = i.fromPos().getY();
	unsigned int eX = i.toPos().getX();
	unsigned int eY = i.toPos().getY();

	if ( i.from().opened() ) ++bX;
	bool excludeLastLine = i.to().opened() && eX == 0;
	if ( i.to().opened() && eX > 0 ) --eX;
	QString bL = mBuffer->textline( bY ).left( bX );
	QString eL = excludeLastLine ? mBuffer->textline( eY ) : mBuffer->textline( eY ).mid( eX + 1 );

	unsigned int cLine = bY+1;
	for( unsigned k = cLine; k <= eY; k++ )
		mBuffer->deleteLine( cLine );
	mBuffer->replaceLine( bL + eL, bY );

#if QT_VERSION < 0x040000
	QValueList<QChar>::const_iterator it = reg.begin(), endd = reg.end();
	for ( ; it != endd; ++it )
		YZSession::mRegisters->setRegister( *it, buff );
#else
	for ( int ab = 0 ; ab < reg.size(); ++ab )
		YZSession::mRegisters->setRegister( reg.at(ab), buff );
#endif

	pView->gotoxyAndStick( bX, bY );

	COMMIT_VIEWS_CHANGES;
}

#if QT_VERSION < 0x040000
void YZAction::copyArea( YZView* pView, const YZCursor& beginCursor, const YZCursor& endCursor, const QValueList<QChar> &reg ) {
#else
void YZAction::copyArea( YZView* pView, const YZCursor& beginCursor, const YZCursor& endCursor, const QList<QChar> &reg ) {
#endif
	YZCursor begin(beginCursor <= endCursor ? beginCursor : endCursor),
		end(beginCursor <= endCursor ? endCursor : beginCursor);
	copyArea( pView, YZInterval(begin, end), reg );
}

#if QT_VERSION < 0x040000
void YZAction::deleteArea( YZView* pView, const YZCursor& beginCursor, const YZCursor& endCursor, const QValueList<QChar> &reg ) {
#else
void YZAction::deleteArea( YZView* pView, const YZCursor& beginCursor, const YZCursor& endCursor, const QList<QChar> &reg ) {
#endif
	YZCursor begin(beginCursor <= endCursor ? beginCursor : endCursor),
		end(beginCursor <= endCursor ? endCursor : beginCursor);
	deleteArea( pView, YZInterval(begin, end), reg );
}

void YZAction::mergeNextLine( YZView* pView, unsigned int y, bool stripSpaces ) {
	CONFIGURE_VIEWS;
	QString line = mBuffer->textline( y );
	QString line2 = mBuffer->textline( y + 1 );
	if ( stripSpaces ) {
		QString space(" ");
		if ( line.endsWith(" ") || line.endsWith("	") )
			space = "";
		line2.replace(QRegExp("^\\s*"),space);
	}
	mBuffer->replaceLine( line + line2, y );
	mBuffer->deleteLine( y + 1 );
	pView->gotoxyAndStick( line.length(), y );
	COMMIT_VIEWS_CHANGES;
}

void YZAction::indentLine( YZView* pView, unsigned int Y, int count ) {
	if ( count == 0 ) return;
	CONFIGURE_VIEWS;
	QString line = mBuffer->textline( Y );
	if ( count > 0 ) {
		QString s;
		s.fill( '\t', count );
		line = s + line;
	} else {
		count = -count;
		unsigned int tabstop = pView->getLocalIntOption( "tabstop" );
		QRegExp reg( "^(\t| {1," + QString::number( tabstop - 1 ) + "}\t?| {" + QString::number( tabstop ) + "})" );
		for ( unsigned int c = count; c > 0; c-- )
			line = line.replace( reg, "" );
	}
	replaceLine( pView, Y, line );
	pView->moveToFirstNonBlankOfLine();
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
#if QT_VERSION < 0x040000
void YZAction::deleteLine( YZView* pView, unsigned int Y, unsigned int len, const QValueList<QChar>& regs ) {
#else
void YZAction::deleteLine( YZView* pView, unsigned int Y, unsigned int len, const QList<QChar>& regs ) {
#endif
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
	QChar cchar = current.at(mCursor.getX());

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
					if ( current.at( j ) == cchar ) {
						count++;
					} else if ( current.at( j ) == c ) {
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
//	yzDebug() << " Searching " << what << " from " << mBegin << " to " << mEnd << " Reverse : " << reverseSearch << endl;
	QRegExp ex( what );

	unsigned int currentMatchLine;
	int currentMatchColumn;
	QString l;
	
	unsigned int i = reverseSearch ? qMin( (int)mBegin.getY(), (int)(pView->myBuffer()->lineCount() - 1) ) 
					: qMax( (int)mBegin.getY(), 0 );
	unsigned int maxLine = reverseSearch ? qMax( (int)mEnd.getY(), 0 ) : 
						qMin( (int)mEnd.getY(), (int)(pView->myBuffer()->lineCount() - 1) );
	for ( ; ( reverseSearch && i >= maxLine ) || ( !reverseSearch && i <= maxLine ) ; reverseSearch ? i-- : i++ ) {
		if ( i == ( unsigned int )( -1 ) ) break; //woups ;)
		l = pView->myBuffer()->textline( i );

		int idx;
		if ( reverseSearch ) {
			currentMatchColumn = -1;
			if ( i == mBegin.getY() ) {
				if ( mBegin.getX() == 0 ) continue;
				currentMatchColumn = mBegin.getX() - 1;
			} else if ( i == mEnd.getY() ) {
				l = l.mid( mEnd.getX() );
			}
#if QT_VERSION < 0x040000
			idx = ex.searchRev( l, currentMatchColumn );
#else
			idx = ex.lastIndexIn( l, currentMatchColumn );
#endif
			if ( i == mBegin.getY() && idx >= (int)mBegin.getX() ) idx = -1;
//			yzDebug() << "searchRev on " << l << " starting at " << currentMatchColumn << " = " << idx << endl;
		} else {
			currentMatchColumn = 0;
			if ( i == mBegin.getY() ) {
				currentMatchColumn = mBegin.getX();
			} else if ( i == mEnd.getY() ) {
				l = l.left( mEnd.getX() );
			}
#if QT_VERSION < 0x040000
			idx = ex.search( l, currentMatchColumn );
#else
			idx = ex.indexIn( l, currentMatchColumn );
#endif
//			yzDebug() << "search on " << l << " starting at " << currentMatchColumn << " = " << idx << endl;
		}

		if ( idx >= 0 ) {
			currentMatchColumn = idx;
			currentMatchLine = i;
			//if we truncated the line below, then we need to change the cursor getX position too ;)
			if ( i == mEnd.getY() && reverseSearch )
				currentMatchColumn += mEnd.getX();
			*found = true;
			*matchlength = ex.matchedLength();
//			yzDebug() << "Search got one result " << endl;
			return YZCursor(pView,currentMatchColumn, currentMatchLine);
		}
	}
	*found = false;
//	yzDebug() << "Search got no result " << endl;
	return YZCursor(pView,0,0);//fake result
}

