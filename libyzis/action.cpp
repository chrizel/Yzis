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
#include <qclipboard.h>
#include <qpaintdevice.h>

YZAction::YZAction( YZBuffer* buffer ) {
	mBuffer = buffer;
}
YZAction::~YZAction( ) {
}

static void configureViews(YZBuffer *buffer)
{
	YZList<YZView*> views = buffer->views();
	for ( YZList<YZView*>::Iterator itr = views.begin(); itr != views.end(); ++itr ) {
		(*itr)->setPaintAutoCommit( false );
	}
}

static void commitViewsChanges(YZBuffer *buffer)
{
	YZList<YZView*> views = buffer->views();
	for ( YZList<YZView*>::Iterator itr = views.begin(); itr != views.end(); ++itr ) {
		(*itr)->commitPaintEvent();
	}
}

void YZAction::insertChar( YZView* pView, const YZCursor& pos, const QString& text ) {
	configureViews(mBuffer);
	if( pos.y() == mBuffer->lineCount() )
		mBuffer->insertNewLine( pos.x(), pos.y() );
	else if (pos.y() > mBuffer->lineCount() )
		return; //can't insert on non existing lines
	mBuffer->insertChar( pos.x(), pos.y(), text );
	pView->gotoxyAndStick( pos.x() + text.length(), pos.y() );
	commitViewsChanges(mBuffer);
}

void YZAction::replaceText( YZView* pView, const YZCursor& pos, unsigned int replacedLength, const QString& text ) {
	yzDebug() << "replaceText :" << pos << " length : " << replacedLength << " text:" << text << endl;
	if( pos.y() >= mBuffer->lineCount() ) 
		return; //dont try on non existing lines
	configureViews(mBuffer);
	mBuffer->delChar( pos.x(), pos.y(), replacedLength );
	mBuffer->insertChar( pos.x(), pos.y(), text );
	pView->gotoxyAndStick( pos.x() + text.length(), pos.y() );
	commitViewsChanges(mBuffer);
}

void YZAction::replaceChar( YZView* pView, const YZCursor& pos, const QString& text ) {
	if( pos.y() >= mBuffer->lineCount() ) 
		return; //dont try on non existing lines
	configureViews(mBuffer);
	mBuffer->delChar( pos.x(), pos.y(), text.length() );
	mBuffer->insertChar( pos.x(), pos.y(), text );
	pView->gotoxyAndStick( pos.x() + text.length(), pos.y() );
	commitViewsChanges(mBuffer);
}

void YZAction::deleteChar( YZView* pView, const YZCursor& pos, unsigned int len ) {
	if( pos.y() >= mBuffer->lineCount() ) 
		return; //dont try on non existing lines
	configureViews(mBuffer);
	mBuffer->delChar( pos.x(), pos.y(), len );
	pView->gotoxyAndStick( pos.x(), pos.y() );
	commitViewsChanges(mBuffer);
}

void YZAction::appendLine( YZView* pView, const QString& text ) {
	configureViews(mBuffer);
	unsigned int y = mBuffer->lineCount();
	mBuffer->insertNewLine( 0, y );
	mBuffer->insertChar( 0, y, text );
	pView->gotoxyAndStick( text.length(), y );
	commitViewsChanges(mBuffer);
}

void YZAction::insertNewLine( YZView* pView, const YZCursor& pos ) {
	if( pos.y() > mBuffer->lineCount() ) 
		return; //dont try on non existing lines
	configureViews(mBuffer);
	mBuffer->insertNewLine( pos.x(), pos.y() );
	pView->gotoxyAndStick( 0, pos.y() + 1 );
	commitViewsChanges(mBuffer);
}

void YZAction::replaceLine( YZView* pView, const YZCursor& pos, const QString &text ) {
	if( pos.y() >= mBuffer->lineCount() ) 
		return; //dont try on non existing lines
	configureViews(mBuffer);
	mBuffer->replaceLine( text, pos.y() );
	pView->gotoxyAndStick( text.length(), pos.y() );
	commitViewsChanges(mBuffer);
}

void YZAction::insertLine( YZView* pView, const YZCursor& pos, const QString &text ) {
	if( pos.y() > mBuffer->lineCount() ) 
		return; //dont try on non existing lines
	configureViews(mBuffer);
	mBuffer->insertLine( text, pos.y() );
	pView->gotoxyAndStick( text.length(), pos.y() );
	commitViewsChanges(mBuffer);
}

void YZAction::deleteLine( YZView* pView, const YZCursor& pos, unsigned int len, const QValueList<QChar> &reg ) {
	configureViews(mBuffer);
	copyLine(pView, pos, len, reg);
	if ( pos.y() + len > mBuffer->lineCount() )
		len = mBuffer->lineCount() - pos.y();
	for ( unsigned int i = 0; i < len && pos.y() < mBuffer->lineCount(); i++ )
		mBuffer->deleteLine( pos.y() );
	pView->gotoxyAndStick( 0, pos.y() - (pos.y() == mBuffer->lineCount() ? 1 : 0) );
	commitViewsChanges(mBuffer);
}

void YZAction::copyLine( YZView* , const YZCursor& pos, unsigned int len, const QValueList<QChar> &reg ) {
	YZCursor mPos( pos );

	unsigned int bY = mPos.y();
	QStringList buff;
	QString text = "";
	QString line;
	buff << QString::null;
	for ( unsigned int i = 0; i < len && (bY + i) < mBuffer->lineCount(); i++ ) {
		line = mBuffer->textline( bY + i );
		buff << line;
		text += line + "\n";
	}
	buff << QString::null;
#ifdef Q_WS_X11
	if ( QPaintDevice::x11AppDisplay() )
#endif
		QApplication::clipboard()->setText( text, QClipboard::Clipboard );
	
	QValueList<QChar>::const_iterator it = reg.begin(), end = reg.end();
	for ( ; it != end; ++it )
		YZSession::mRegisters->setRegister( *it, buff );
}


void YZAction::copyArea( YZView* /*pView*/, const YZInterval& i, const QValueList<QChar> &reg ) {
	QStringList buff;
	unsigned int bX = i.fromPos().x();
	unsigned int bY = i.fromPos().y();
	unsigned int eX = i.toPos().x();
	unsigned int eY = i.toPos().y();

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
		if ( eY < mBuffer->lineCount() )
			buff << mBuffer->textline( eY ).left( eX );
	}

#ifdef Q_WS_X11
	if ( QPaintDevice::x11AppDisplay() )
#endif
		QApplication::clipboard()->setText( mBuffer->getText( i ).join("\n"), QClipboard::Clipboard );
	
	yzDebug() << "Copied " << buff << endl;
	QValueList<QChar>::const_iterator it = reg.begin(), endd = reg.end();
	for ( ; it != endd; ++it )
		YZSession::mRegisters->setRegister( *it, buff );


}

void YZAction::replaceArea( YZView* /*pView*/, const YZInterval& i, const QStringList& text ) {
	configureViews(mBuffer);
	QStringList t = text;

	unsigned int bX = i.fromPos().x();
	unsigned int bY = i.fromPos().y();
	unsigned int eX = i.toPos().x();
	unsigned int eY = i.toPos().y();

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
	
	commitViewsChanges(mBuffer);
}

void YZAction::deleteArea( YZView* pView, const YZInterval& i, const QValueList<QChar> &reg ) {
	yzDebug() << "YZAction::deleteArea " << i << endl;
	configureViews(mBuffer);

	QStringList buff = mBuffer->getText( i );

	unsigned int bX = i.fromPos().x();
	unsigned int bY = i.fromPos().y();
	unsigned int eX = i.toPos().x();
	unsigned int eY = i.toPos().y();

	if ( i.from().opened() ) ++bX;
	bool excludeLastLine = i.to().opened() && eX == 0;
	bool deleteAfterEnd = excludeLastLine && eY >= mBuffer->lineCount();
	if ( i.to().opened() && eX > 0 ) --eX;

	QString bL = mBuffer->textline( bY ).left( bX );
	QString eL;
	if ( deleteAfterEnd ) {
		eY = mBuffer->lineCount() - 1;
		if ( bY > 0 )
			eL = mBuffer->textline( bY - 1 );
		else
			eL = "";
	} else if ( excludeLastLine )
		eL = mBuffer->textline( eY );
	else
		eL = mBuffer->textline( eY ).mid( eX + 1 );

	unsigned int cLine = bY + 1;
	for( unsigned k = cLine; k <= eY; k++ )
		mBuffer->deleteLine( cLine );
	mBuffer->replaceLine( bL + eL, bY );

	QValueList<QChar>::const_iterator it = reg.begin(), endd = reg.end();
	for ( ; it != endd; ++it )
		YZSession::mRegisters->setRegister( *it, buff );

	pView->gotoxyAndStick( bX, bY );

	commitViewsChanges(mBuffer);
}

void YZAction::copyArea( YZView* pView, const YZCursor& beginCursor, const YZCursor& endCursor, const QValueList<QChar> &reg ) {
	YZCursor begin(beginCursor <= endCursor ? beginCursor : endCursor),
		end(beginCursor <= endCursor ? endCursor : beginCursor);
	copyArea( pView, YZInterval(begin, end), reg );
}

void YZAction::deleteArea( YZView* pView, const YZCursor& beginCursor, const YZCursor& endCursor, const QValueList<QChar> &reg ) {
	YZCursor begin(beginCursor <= endCursor ? beginCursor : endCursor),
		end(beginCursor <= endCursor ? endCursor : beginCursor);
	deleteArea( pView, YZInterval(begin, end), reg );
}

void YZAction::mergeNextLine( YZView* pView, unsigned int y, bool stripSpaces ) {
	if ( y >= mBuffer->lineCount() - 1 ) return;
	configureViews(mBuffer);
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
	commitViewsChanges(mBuffer);
}

void YZAction::indentLine( YZView* pView, unsigned int Y, int count ) {
	if ( count == 0 ) return;
	configureViews(mBuffer);
	QString line = mBuffer->textline( Y );
	if ( count > 0 ) {
		QString s;
		s.fill( '\t', count );
		line = s + line;
	} else {
		count = -count;
		unsigned int tabstop = pView->getLocalIntegerOption( "tabstop" );
		QRegExp reg( "^(\t| {1," + QString::number( tabstop - 1 ) + "}\t?| {" + QString::number( tabstop ) + "})" );
		for ( unsigned int c = count; c > 0; c-- )
			line = line.replace( reg, "" );
	}
	replaceLine( pView, Y, line );
	pView->moveToFirstNonBlankOfLine();
	commitViewsChanges(mBuffer);
}


void YZAction::insertChar( YZView* pView, unsigned int X, unsigned int Y, const QString& text ) {
	YZCursor pos( X, Y );
	insertChar( pView, pos, text );
}
void YZAction::replaceChar( YZView* pView, unsigned int X, unsigned int Y, const QString& text ) {
	YZCursor pos( X, Y );
	replaceChar( pView, pos, text );
}
void YZAction::deleteChar( YZView* pView, unsigned int X, unsigned int Y, unsigned int len ) {
	YZCursor pos( X, Y );
	deleteChar( pView, pos, len );
}
void YZAction::insertLine( YZView* pView, unsigned int Y, const QString &text ) {
	YZCursor pos( 0, Y );
	insertLine( pView, pos, text );
}
void YZAction::insertNewLine( YZView* pView, unsigned int X, unsigned int Y ) {
	YZCursor pos( X, Y );
	insertNewLine( pView, pos );
}
void YZAction::deleteLine( YZView* pView, unsigned int Y, unsigned int len, const QValueList<QChar>& regs ) {
	YZCursor pos( 0, Y );
	deleteLine( pView, pos, len, regs );
}
void YZAction::replaceLine( YZView* pView, unsigned int Y, const QString& text ) {
	YZCursor pos( 0, Y );
	replaceLine( pView, pos, text );
}

YZCursor YZAction::match( YZView* pView, YZCursor& mCursor, bool *found ) {
	QString matchers = pView->myBuffer()->getLocalStringOption("matchpairs");

	QString current = pView->myBuffer()->textline( mCursor.y() );
	QChar cchar = current.at(mCursor.x());

	int i = 0;
	unsigned int j = 0;
	unsigned int curY = mCursor.y();
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
				if ( back && mCursor.y() == curY ) {
					if ( mCursor.x() == 0) {
						curY--;
						current = pView->myBuffer()->textline( curY );
						start = current.length() - 1;
					} else
						start = mCursor.x()-1;
				} else if ( !back && mCursor.y() == curY )
					start =  mCursor.x()+1;
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
		return YZCursor( ( back ? j + 1 : j - 1 ), curY );
	}
	*found=false;
	return YZCursor( 0,0 );
}

//mBegin is always the beginning of the search so if reverseSearch is true , we have mEnd < mBegin ;)
// which makes reverseSearch redundant.  It's now calculated within the function based on a test of mEnd < mBegin
YZCursor YZAction::search( YZBuffer* pBuffer, const QString& _what, const YZCursor& mBegin, const YZCursor& mEnd, unsigned int *matchlength, bool *found ) {
//	yzDebug() << " Searching " << _what << " from " << mBegin << " to " << mEnd << " Reverse : " << reverseSearch << endl;
	bool reverseSearch = mEnd < mBegin;
	bool cs = true;
	QString what = _what;
	if ( what.endsWith("\\c") ) {
		what.truncate(what.length()-2);
		cs = false;
	}
//	yzDebug() << " Casesensitive : " << cs << endl;
	QRegExp ex( what );
	ex.setCaseSensitive(cs);

	unsigned int currentMatchLine;
	int currentMatchColumn;
	QString l;
	
	unsigned int i = reverseSearch ? qMin( (int)mBegin.y(), (int)(pBuffer->lineCount() - 1) ) 
					: qMax( (int)mBegin.y(), 0 );
	unsigned int maxLine = reverseSearch ? qMax( (int)mEnd.y(), 0 ) : 
						qMin( (int)mEnd.y(), (int)(pBuffer->lineCount() - 1) );
	for ( ; ( reverseSearch && i >= maxLine ) || ( !reverseSearch && i <= maxLine ) ; reverseSearch ? i-- : i++ ) {
		if ( i == ( unsigned int )( -1 ) ) break; //woups ;)
		l = pBuffer->textline( i );

		int idx;
		if ( reverseSearch ) {
			currentMatchColumn = -1;
			if ( i == mBegin.y() ) {
				if ( mBegin.x() == 0 ) continue;
				currentMatchColumn = mBegin.x() - 1;
			} else if ( i == mEnd.y() ) {
				l = l.mid( mEnd.x() );
			}
			idx = ex.searchRev( l, currentMatchColumn );
			if ( i == mBegin.y() && idx >= (int)mBegin.x() ) idx = -1;
//			yzDebug() << "searchRev on " << l << " starting at " << currentMatchColumn << " = " << idx << endl;
		} else {
			currentMatchColumn = 0;
			if ( i == mBegin.y() ) {
				currentMatchColumn = mBegin.x();
			} else if ( i == mEnd.y() ) {
				l = l.left( mEnd.x() );
			}
			idx = ex.search( l, currentMatchColumn );
//			yzDebug() << "search on " << l << " starting at " << currentMatchColumn << " = " << idx << endl;
		}

		if ( idx >= 0 ) {
			currentMatchColumn = idx;
			currentMatchLine = i;
			//if we truncated the line below, then we need to change the cursor x position too ;)
			if ( i == mEnd.y() && reverseSearch )
				currentMatchColumn += mEnd.x();
			*found = true;
			*matchlength = ex.matchedLength();
//			yzDebug() << "Search got one result " << endl;
			return YZCursor(currentMatchColumn, currentMatchLine);
		}
	}
	*found = false;
//	yzDebug() << "Search got no result " << endl;
	return YZCursor(0,0);//fake result
}

