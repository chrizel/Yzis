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

YZAction::YZAction( YBuffer* buffer )
{
    dbg() << "YZAction(" << buffer->toString() << ")" << endl;
    mBuffer = buffer;
}

YZAction::~YZAction( )
{
    dbg() << "~YZAction()" << endl;
}

static void configureViews(YBuffer *buffer)
{
    dbg() << "configureViews(" << buffer->toString() << ")" << endl;
    foreach( YView *view, buffer->views() )
                view->setPaintAutoCommit( false );
}

static void commitViewsChanges(YBuffer *buffer)
{
    dbg() << "commitViewsChanges(" << buffer->toString() << ")" << endl;
    foreach( YView *view, buffer->views() )
                view->commitPaintEvent();
}

void YZAction::insertChar( YView* pView, const YCursor pos, const QString& text )
{
    dbg() << "insertChar(" << pView->toString() << "," << pos << ", " << text << ")" << endl;
    configureViews(mBuffer);
    if ( pos.y() == mBuffer->lineCount() )
        mBuffer->insertNewLine( pos );
    else if (pos.y() > mBuffer->lineCount() )
        return ; // can't insert on non-existing lines
    mBuffer->insertChar( pos, text );
    pView->gotoLinePositionAndStick(pos.y() , pos.x() + text.length());
    commitViewsChanges(mBuffer);
}

void YZAction::replaceText( YView* pView, const YCursor pos, int replacedLength, const QString& text )
{
    dbg() << "replaceText(" << pView->toString() << ", pos, " << replacedLength << "," << text << ")" << endl;
    if ( pos.y() >= mBuffer->lineCount() )
        return ; // don't try on non-existing lines
    configureViews(mBuffer);
    mBuffer->delChar( pos, replacedLength );
    mBuffer->insertChar( pos, text );
    pView->gotoLinePositionAndStick(pos.y() , pos.x() + text.length());
    commitViewsChanges(mBuffer);
}

bool YZAction::replaceChar( YView* pView, const YCursor pos, const QString& text )
{
	YASSERT(pos.y() < mBuffer->lineCount());
    configureViews(mBuffer);
    mBuffer->delChar( pos, text.length() );
    mBuffer->insertChar( pos, text );
    pView->gotoLinePositionAndStick(pos.y() , pos.x() + text.length());
    commitViewsChanges(mBuffer);
    return false;
}

bool YZAction::deleteChar( YView* pView, const YCursor pos, int len )
{
	YASSERT(pos.y() < mBuffer->lineCount());
    configureViews(mBuffer);
    mBuffer->delChar( pos, len );
    pView->gotoLinePositionAndStick( pos );
    commitViewsChanges(mBuffer);
    return false;
}

void YZAction::insertNewLine( YView* pView, const YCursor pos )
{
	YASSERT(pos.y() < mBuffer->lineCount());
    configureViews(mBuffer);
    mBuffer->insertNewLine( pos );
    pView->gotoLinePositionAndStick(pos.y() + 1 , 0);
    commitViewsChanges(mBuffer);
}

void YZAction::replaceLine( YView* pView, const YCursor pos, const QString &text )
{
	YASSERT(pos.y() < mBuffer->lineCount());
    configureViews(mBuffer);
    mBuffer->replaceLine( text, pos.y() );
    pView->gotoLinePositionAndStick(pos.y() , text.length());
    commitViewsChanges(mBuffer);
}

void YZAction::ensureLineExists( int line ) {
    configureViews(mBuffer);
	if ( line >= mBuffer->lineCount() ) {
		int l = mBuffer->lineCount() - 1;
		mBuffer->insertNewLine(YCursor(mBuffer->getLineLength(l),l));
		for ( ++l; l < line; ++l )
			mBuffer->insertNewLine(YCursor(0,l));
	}
    commitViewsChanges(mBuffer);
}

void YZAction::insertLine( YView* pView, const YCursor pos, const QString &text )
{
    if ( pos.y() > mBuffer->lineCount() )
        return ; // don't try on non-existing lines
    configureViews(mBuffer);
    mBuffer->insertLine( text, pos.y() );
    pView->gotoLinePositionAndStick(pos.y() , text.length());
    commitViewsChanges(mBuffer);
}

void YZAction::deleteLine( YView* pView, const YCursor pos, int len, const QList<QChar> &reg )
{
    configureViews(mBuffer);
    copyLine(pView, pos, len, reg);
    if ( pos.y() + len > mBuffer->lineCount() )
        len = mBuffer->lineCount() - pos.y();
    for ( int i = 0; i < len && pos.y() < mBuffer->lineCount(); i++ )
        mBuffer->deleteLine( pos.y() );
    pView->gotoLinePositionAndStick(pos.y() - (pos.y() == mBuffer->lineCount() ? 1 : 0) , 0);
    commitViewsChanges(mBuffer);
}

void YZAction::copyLine( YView* , const YCursor pos, int len, const QList<QChar> &reg )
{
    YCursor mPos( pos );

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
    YSession::self()->guiSetClipboardText( text, Clipboard::Clipboard );
    for ( int ab = 0 ; ab < reg.size(); ++ab )
        YSession::self()->setRegister( reg.at(ab), buff );
}


void YZAction::copyArea( YView* , const YInterval& i, const QList<QChar> &reg )
{
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
        for ( int y = bY + 1; y < eY; y++ )
            buff << mBuffer->textline( y );
        if ( eY < mBuffer->lineCount() )
            buff << mBuffer->textline( eY ).left( eX );
    }

	YSession::self()->guiSetClipboardText(mBuffer->dataRegion(i).join("\n"), Clipboard::Clipboard);

    dbg() << "Copied " << buff << endl;
    for ( int ab = 0 ; ab < reg.size(); ++ab )
        YSession::self()->setRegister( reg.at(ab), buff );
}

void YZAction::replaceArea( YView* /*pView*/, const YInterval& i, const YRawData& text )
{
	mBuffer->replaceRegion(i, text);
}

void YZAction::deleteArea( YView* pView, const YInterval& i, const QList<QChar> &reg )
{
    dbg() << "YZAction::deleteArea " << i << endl;
    configureViews(mBuffer);

	YRawData buff = mBuffer->dataRegion(i);

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
    for ( int k = cLine; k <= eY; k++ )
        mBuffer->deleteLine( cLine );
    mBuffer->replaceLine( bL + eL, bY );

    for ( int ab = 0 ; ab < reg.size(); ++ab )
        YSession::self()->setRegister( reg.at(ab), buff );

    pView->gotoLinePositionAndStick(bY , bX);

    commitViewsChanges(mBuffer);
}

void YZAction::copyArea( YView* pView, const YCursor beginCursor, const YCursor endCursor, const QList<QChar> &reg )
{
    YCursor begin(beginCursor <= endCursor ? beginCursor : endCursor),
    end(beginCursor <= endCursor ? endCursor : beginCursor);
    copyArea( pView, YInterval(begin, end), reg );
}

void YZAction::deleteArea( YView* pView, const YCursor beginCursor, const YCursor endCursor, const QList<QChar> &reg )
{
    YCursor begin(beginCursor <= endCursor ? beginCursor : endCursor),
    end(beginCursor <= endCursor ? endCursor : beginCursor);
    deleteArea( pView, YInterval(begin, end), reg );
}

void YZAction::mergeNextLine( YView* pView, int y, bool stripSpaces )
{
    if ( y >= mBuffer->lineCount() - 1 ) return ;
    configureViews(mBuffer);
    QString line = mBuffer->textline( y );
    QString line2 = mBuffer->textline( y + 1 );
    if ( stripSpaces ) {
        QString space(" ");
        if ( line.endsWith(" ") || line.endsWith(" ") )
            space = "";
        line2.replace(QRegExp("^\\s*"), space);
    }
    mBuffer->replaceLine( line + line2, y );
    mBuffer->deleteLine( y + 1 );
    pView->gotoLinePositionAndStick(y , line.length());
    commitViewsChanges(mBuffer);
}

void YZAction::indentLine( YView* pView, int Y, int count )
{
    if ( count == 0 ) return ;
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
    pView->gotoViewCursor(pView->viewCursorFromLinePosition(Y, pView->buffer()->firstNonBlankChar(Y)));
    commitViewsChanges(mBuffer);
}

YCursor YZAction::match( YView* pView, const YCursor cursor, bool *found ) const
{
    QString matchers = pView->buffer()->getLocalStringOption("matchpairs");

    QString current = pView->buffer()->textline( cursor.y() );
    QChar cchar = current.at(cursor.x());

    int i = 0;
    int j = 0;
    int curY = cursor.y();
    int count = 1;
    bool back = false;
    int start = 0;

    for ( i = 0; i < ( int )matchers.length() ; i++ ) {
        if ( matchers[ i ] == cchar ) {
            back = ( abs( i / 2 ) * 2 ) != i;
            QChar c = matchers[ back ? i - 1 : i + 1 ]; //the character to match
            //now do the real search
            while ( curY < pView->buffer()->lineCount() && count > 0 ) {
                current = pView->buffer()->textline( curY );
                if ( back && cursor.y() == curY ) {
                    if ( cursor.x() == 0) {
                        curY--;
                        current = pView->buffer()->textline( curY );
                        start = current.length() - 1;
                    } else
                        start = cursor.x() - 1;
                } else if ( !back && cursor.y() == curY )
                    start = cursor.x() + 1;
                else
                    start = back ? current.length() - 1 : 0 ;

                for ( j = start; ( j < current.length() ) && ( count > 0 ) && (j >= 0) ; back ? j-- : j++ ) { //parse current line
                    if ( current.at( j ) == cchar ) {
                        count++;
                    } else if ( current.at( j ) == c ) {
                        count--; // we need to skip one more
                    }
                }
                if ( count > 0 ) { //let's do another loop
                    //current = pView->buffer()->textline( back ? --curY : ++curY );
                    if ( back ) --curY;
                    else curY++;
                }
            }
        }
    }
    if ( count == 0 ) { //found it !
        *found = true;
        dbg() << "Result action: " << ( back ? j + 1 : j - 1 ) << ", " << curY << endl;
        return YCursor( ( back ? j + 1 : j - 1 ), curY );
    }
    *found = false;
    return YCursor( 0, 0 );
}

//mBegin is always the beginning of the search so if reverseSearch is true , we have mEnd < mBegin ;)
// which makes reverseSearch redundant.  It's now calculated within the function based on a test of mEnd < mBegin

YCursor YZAction::search( YBuffer* pBuffer, const QString& _what, const YCursor mBegin, const YCursor mEnd, int *matchlength, bool *found ) const
{
    // dbg() << " Searching " << _what << " from " << mBegin << " to " << mEnd << " Reverse : " << reverseSearch << endl;
    bool reverseSearch = mEnd < mBegin;
    bool cs = true;
    QString what = _what;
    if ( what.endsWith("\\c") ) {
        what.truncate(what.length() - 2);
        cs = false;
    }
    // dbg() << " Casesensitive: " << cs << endl;
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
            //   dbg() << "searchRev on " << l << " starting at " << currentMatchColumn << " = " << idx << endl;
        } else {
            currentMatchColumn = 0;
            if ( i == mBegin.y() ) {
                currentMatchColumn = mBegin.x();
            } else if ( i == mEnd.y() ) {
                l = l.left( mEnd.x() );
            }
            idx = ex.indexIn( l, currentMatchColumn );
            //   dbg() << "search on " << l << " starting at " << currentMatchColumn << " = " << idx << endl;
        }

        if ( idx >= 0 ) {
            currentMatchColumn = idx;
            currentMatchLine = i;
            //if we truncated the line below, then we need to change the cursor x position too ;)
            if ( i == mEnd.y() && reverseSearch )
                currentMatchColumn += mEnd.x();
            *found = true;
            *matchlength = ex.matchedLength();
            //   dbg() << "Search got one result " << endl;
            return YCursor(currentMatchColumn, currentMatchLine);
        }
    }
    *found = false;
    // dbg() << "Search got no result " << endl;
    return YCursor(0, 0); //fake result
}

void YZAction::pasteContent( YView *view, QChar registr, bool after )
{
    QStringList list = YSession::self()->getRegister( registr );
    if ( list.isEmpty() ) return ;

    YCursor pos( view->viewCursor().buffer() );
    int i = 0;
    bool copyWholeLinesOnly = list[ 0 ].isNull();
    QString copy = mBuffer->textline( pos.y() );
    if ( after || ! copyWholeLinesOnly ) { //paste after current char
        int start;
        if ( after )
            start = copy.length() > 0 ? pos.x() + 1 : 0;
        else
            start = pos.x();
        i = 0;
        if ( ! copyWholeLinesOnly ) {
            copy = copy.mid( start );
            deleteChar( view, start, pos.y(), copy.length() );
            insertChar( view, start, pos.y(), list[ 0 ] + ( list.size() == 1 ? copy : "" ) );
            view->gotoLinePosition(pos.y() , start + list[ 0 ].length() - ( list[ 0 ].length() > 0 ? 1 : 0 ));
        }
        i++;
        while ( i < list.size() - 1 ) {
                        mBuffer->insertLine( list[ i ], pos.y() + i );
            i++;
        }
        if ( i < list.size() && ! copyWholeLinesOnly ) {
            configureViews(mBuffer);
            mBuffer->insertLine( (list[ i ].isNull() ? "" : list[ i ] ) + copy, pos.y() + i );
            commitViewsChanges(mBuffer);
            view->gotoLinePosition(pos.y() + i , list[ i ].length());
        } else if ( copyWholeLinesOnly ) {
            view->gotoLinePosition(pos.y() + 1 , view->buffer()->firstNonBlankChar(pos.y()+1));
        }

    } else if ( !after ) { //paste whole lines before current char
        configureViews(mBuffer);
        for ( i = 1; i < list.size() - 1; i++ )
                mBuffer->insertLine( list[ i ], pos.y() + i - 1 );
           commitViewsChanges(mBuffer);

        view->gotoLinePosition( pos );
    }
    view->stickToColumn();
}
