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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

   
/* Yzis */
#include "action.h"
#include "view.h"
#include "debug.h"
#include "buffer.h"
#include "session.h"

#define dbg()    yzDebug("YZAction")
#define err()    yzError("YZAction")

YZAction::YZAction( YZBuffer* buffer ) {
    dbg() << "YZAction(" << buffer->toString() << ")" << endl;
	mBuffer = buffer;
}

YZAction::~YZAction( ) {
    dbg() << "~YZAction()" << endl;
}

static void configureViews(YZBuffer *buffer)
{
    dbg() << "configureViews(" << buffer->toString() << ")" << endl;
	foreach( YZView *view, buffer->views() )
		view->setPaintAutoCommit( false );
}

static void commitViewsChanges(YZBuffer *buffer)
{
    dbg() << "commitViewsChanges(" << buffer->toString() << ")" << endl;
	foreach( YZView *view, buffer->views() )
		view->commitPaintEvent();
}

void YZAction::insertChar( YZView* pView, const YZCursor pos, const QString& text ) {
    dbg() << "insertChar(" << pView->toString() << ", pos, " << text << ")" << endl;
	configureViews(mBuffer);
	if( pos.y() == mBuffer->lineCount() )
		mBuffer->insertNewLine( pos.x(), pos.y() );
	else if (pos.y() > mBuffer->lineCount() )
		return; // can't insert on non-existing lines
	mBuffer->insertChar( pos, text );
	pView->gotoxyAndStick( pos.x() + text.length(), pos.y() );
	commitViewsChanges(mBuffer);
}

void YZAction::replaceText( YZView* pView, const YZCursor pos, int replacedLength, const QString& text ) {
    dbg() << "replaceText(" << pView->toString() << ", pos, " << replacedLength << "," << text << ")" << endl;
	if( pos.y() >= mBuffer->lineCount() ) 
		return; // don't try on non-existing lines
	configureViews(mBuffer);
	mBuffer->delChar( pos, replacedLength );
	mBuffer->insertChar( pos, text );
	pView->gotoxyAndStick( pos.x() + text.length(), pos.y() );
	commitViewsChanges(mBuffer);
}

void YZAction::replaceChar( YZView* pView, const YZCursor pos, const QString& text ) {
	if( pos.y() >= mBuffer->lineCount() ) 
		return; // don't try on non-existing lines
	configureViews(mBuffer);
	mBuffer->delChar( pos, text.length() );
	mBuffer->insertChar( pos, text );
	pView->gotoxyAndStick( pos.x() + text.length(), pos.y() );
	commitViewsChanges(mBuffer);
}

void YZAction::deleteChar( YZView* pView, const YZCursor pos, int len ) {
	if( pos.y() >= mBuffer->lineCount() ) 
		return; // don't try on non-existing lines
	configureViews(mBuffer);
	mBuffer->delChar( pos, len );
	pView->gotoxyAndStick( pos );
	commitViewsChanges(mBuffer);
}

void YZAction::appendLine( YZView* pView, const QString& text ) {
	configureViews(mBuffer);
	int y = mBuffer->lineCount();
	mBuffer->insertNewLine( 0, y );
	mBuffer->insertChar( QPoint(0,y), text );
	pView->gotoxyAndStick( text.length(), y );
	commitViewsChanges(mBuffer);
}

void YZAction::insertNewLine( YZView* pView, const YZCursor pos ) {
	if( pos.y() > mBuffer->lineCount() ) 
		return; // don't try on non-existing lines
	configureViews(mBuffer);
	mBuffer->insertNewLine( pos.x(), pos.y() );
	pView->gotoxyAndStick( 0, pos.y() + 1 );
	commitViewsChanges(mBuffer);
}

void YZAction::replaceLine( YZView* pView, const YZCursor pos, const QString &text ) {
	if( pos.y() >= mBuffer->lineCount() ) 
		return; // don't try on non-existing lines
	configureViews(mBuffer);
	mBuffer->replaceLine( text, pos.y() );
	pView->gotoxyAndStick( text.length(), pos.y() );
	commitViewsChanges(mBuffer);
}

void YZAction::insertLine( YZView* pView, const YZCursor pos, const QString &text ) {
	if( pos.y() > mBuffer->lineCount() ) 
		return; // don't try on non-existing lines
	configureViews(mBuffer);
	mBuffer->insertLine( text, pos.y() );
	pView->gotoxyAndStick( text.length(), pos.y() );
	commitViewsChanges(mBuffer);
}

void YZAction::deleteLine( YZView* pView, const YZCursor pos, int len, const QList<QChar> &reg ) {
	configureViews(mBuffer);
	copyLine(pView, pos, len, reg);
	if ( pos.y() + len > mBuffer->lineCount() )
		len = mBuffer->lineCount() - pos.y();
	for ( int i = 0; i < len && pos.y() < mBuffer->lineCount(); i++ )
		mBuffer->deleteLine( pos.y() );
	pView->gotoxyAndStick( 0, pos.y() - (pos.y() == mBuffer->lineCount() ? 1 : 0) );
	commitViewsChanges(mBuffer);
}

void YZAction::copyLine( YZView* , const YZCursor pos, int len, const QList<QChar> &reg ) {
	YZCursor mPos( pos );

	int bY = mPos.y();
	QStringList buff;
	QString text = "";
	QString line;
	buff << QString::null;
	for ( int i = 0; i < len && (bY + i) < mBuffer->lineCount(); i++ ) {
		line = mBuffer->textline( bY + i );
		buff << line;
		text += line + '\n';
	}
	buff << QString::null;
	YZSession::self()->guiSetClipboardText( text, Clipboard::Clipboard );
	for ( int ab = 0 ; ab < reg.size(); ++ab )
		YZSession::self()->setRegister( reg.at(ab), buff );
}


void YZAction::copyArea( YZView* , const YZInterval& i, const QList<QChar> &reg ) {
	QStringList buff;
	int bX = i.fromPos().x();
	int bY = i.fromPos().y();
	int eX = i.toPos().x();
	int eY = i.toPos().y();

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
		for ( int y = bY+1; y < eY; y++ )
			buff << mBuffer->textline( y );
		if ( eY < mBuffer->lineCount() )
			buff << mBuffer->textline( eY ).left( eX );
	}

	YZSession::self()->guiSetClipboardText( mBuffer->getText( i ).join("\n"), Clipboard::Clipboard );
	
	dbg() << "Copied " << buff << endl;
	for ( int ab = 0 ; ab < reg.size(); ++ab )
		YZSession::self()->setRegister( reg.at(ab), buff );
}

void YZAction::replaceArea( YZView* /*pView*/, const YZInterval& i, const QStringList& text ) {
	configureViews(mBuffer);
	QStringList t = text;

	int bX = i.fromPos().x();
	int bY = i.fromPos().y();
	int eX = i.toPos().x();
	int eY = i.toPos().y();

	if ( i.from().opened() ) ++bX;
	if ( i.to().opened() && eX > 0 ) --eX;
	if ( i.to().opened() && eX == 0 ) {
		--eY;
		eX = mBuffer->textline( eY ).length() - 1;
	}
	QString bL = mBuffer->textline( bY ).left( bX );
	QString eL = mBuffer->textline( eY ).mid( eX + 1 );

	int rH = text.size();
	int dH = eY - bY + 1;
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
	int j;
	int max = qMin( rH, dH );
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

void YZAction::deleteArea( YZView* pView, const YZInterval& i, const QList<QChar> &reg ) {
	dbg() << "YZAction::deleteArea " << i << endl;
	configureViews(mBuffer);

	QStringList buff = mBuffer->getText( i );

	int bX = i.fromPos().x();
	int bY = i.fromPos().y();
	int eX = i.toPos().x();
	int eY = i.toPos().y();

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

	int cLine = bY + 1;
	for( int k = cLine; k <= eY; k++ )
		mBuffer->deleteLine( cLine );
	mBuffer->replaceLine( bL + eL, bY );

	for ( int ab = 0 ; ab < reg.size(); ++ab )
		YZSession::self()->setRegister( reg.at(ab), buff );

	pView->gotoxyAndStick( bX, bY );

	commitViewsChanges(mBuffer);
}

void YZAction::copyArea( YZView* pView, const YZCursor beginCursor, const YZCursor endCursor, const QList<QChar> &reg ) {
	YZCursor begin(beginCursor <= endCursor ? beginCursor : endCursor),
		end(beginCursor <= endCursor ? endCursor : beginCursor);
	copyArea( pView, YZInterval(begin, end), reg );
}

void YZAction::deleteArea( YZView* pView, const YZCursor beginCursor, const YZCursor endCursor, const QList<QChar> &reg ) {
	YZCursor begin(beginCursor <= endCursor ? beginCursor : endCursor),
		end(beginCursor <= endCursor ? endCursor : beginCursor);
	deleteArea( pView, YZInterval(begin, end), reg );
}

void YZAction::mergeNextLine( YZView* pView, int y, bool stripSpaces ) {
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

void YZAction::indentLine( YZView* pView, int Y, int count ) {
	if ( count == 0 ) return;
	configureViews(mBuffer);
	QString line = mBuffer->textline( Y );
	if ( count > 0 ) {
		QString s;
		s.fill( '\t', count );
		line = s + line;
	} else {
		count = -count;
		int tabstop = pView->getLocalIntegerOption( "tabstop" );
		QRegExp reg( "^(\t| {1," + QString::number( tabstop - 1 ) + "}\t?| {" + QString::number( tabstop ) + "})" );
		for ( int c = count; c > 0; c-- )
			line = line.replace( reg, "" );
	}
	replaceLine( pView, Y, line );
	pView->moveToFirstNonBlankOfLine();
	commitViewsChanges(mBuffer);
}

YZCursor YZAction::match( YZView* pView, const YZCursor cursor, bool *found ) const {
	QString matchers = pView->myBuffer()->getLocalStringOption("matchpairs");

	QString current = pView->myBuffer()->textline( cursor.y() );
	QChar cchar = current.at(cursor.x());

	int i = 0;
	int j = 0;
	int curY = cursor.y();
	int count = 1;
	bool back = false;
	int start = 0;

	for ( i = 0; i < ( int )matchers.length() ; i++ ) {
		if ( matchers[ i ] == cchar ) {
			back = ( abs( i/2 ) * 2 ) != i;
			QChar c = matchers[ back ? i - 1 : i + 1 ]; //the character to match
			//now do the real search
			while ( curY < pView->myBuffer()->lineCount() && count > 0 ) {
				current = pView->myBuffer()->textline( curY );
				if ( back && cursor.y() == curY ) {
					if ( cursor.x() == 0) {
						curY--;
						current = pView->myBuffer()->textline( curY );
						start = current.length() - 1;
					} else
						start = cursor.x()-1;
				} else if ( !back && cursor.y() == curY )
					start =  cursor.x()+1;
				else
					start = back ? current.length() -1 : 0 ;

				for ( j = start; ( j < current.length() ) && ( count > 0 ) && (j>=0) ; back ? j-- : j++ ) { //parse current line
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
		dbg() << "Result action: " << ( back ? j+1 : j-1 ) << ", " << curY << endl;
		return YZCursor( ( back ? j + 1 : j - 1 ), curY );
	}
	*found=false;
	return YZCursor( 0,0 );
}

//mBegin is always the beginning of the search so if reverseSearch is true , we have mEnd < mBegin ;)
// which makes reverseSearch redundant.  It's now calculated within the function based on a test of mEnd < mBegin

YZCursor YZAction::search( YZBuffer* pBuffer, const QString& _what, const YZCursor mBegin, const YZCursor mEnd, int *matchlength, bool *found ) const {
//	dbg() << " Searching " << _what << " from " << mBegin << " to " << mEnd << " Reverse : " << reverseSearch << endl;
	bool reverseSearch = mEnd < mBegin;
	bool cs = true;
	QString what = _what;
	if ( what.endsWith("\\c") ) {
		what.truncate(what.length()-2);
		cs = false;
	}
//	dbg() << " Casesensitive: " << cs << endl;
	QRegExp ex( what );
	ex.setCaseSensitivity(cs ? Qt::CaseSensitive : Qt::CaseInsensitive );

	int currentMatchLine;
	int currentMatchColumn;
	QString l;
	
	int i = reverseSearch ? qMin( mBegin.y(), pBuffer->lineCount() - 1 ) : qMax( mBegin.y(), 0 );
	int maxLine = reverseSearch ? qMax( mEnd.y(), 0 ) : qMin( mEnd.y(), pBuffer->lineCount() - 1 );
	for ( ; ( reverseSearch && i >= maxLine ) || ( !reverseSearch && i <= maxLine ) ; reverseSearch ? i-- : i++ ) {
		if ( i == -1 ) break; //woups ;)
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
			idx = ex.lastIndexIn( l, currentMatchColumn );
			if ( i == mBegin.y() && idx >= mBegin.x() ) idx = -1;
//			dbg() << "searchRev on " << l << " starting at " << currentMatchColumn << " = " << idx << endl;
		} else {
			currentMatchColumn = 0;
			if ( i == mBegin.y() ) {
				currentMatchColumn = mBegin.x();
			} else if ( i == mEnd.y() ) {
				l = l.left( mEnd.x() );
			}
			idx = ex.indexIn( l, currentMatchColumn );
//			dbg() << "search on " << l << " starting at " << currentMatchColumn << " = " << idx << endl;
		}

		if ( idx >= 0 ) {
			currentMatchColumn = idx;
			currentMatchLine = i;
			//if we truncated the line below, then we need to change the cursor x position too ;)
			if ( i == mEnd.y() && reverseSearch )
				currentMatchColumn += mEnd.x();
			*found = true;
			*matchlength = ex.matchedLength();
//			dbg() << "Search got one result " << endl;
			return YZCursor(currentMatchColumn, currentMatchLine);
		}
	}
	*found = false;
//	dbg() << "Search got no result " << endl;
	return YZCursor(0,0);//fake result
}

