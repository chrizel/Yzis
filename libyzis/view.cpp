/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>.
*  Copyright (C) 2003-2006 Loic Pauleve <panard@inzenet.org>
*  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
*  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
*  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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
#include "view.h"
#include "viewcursor.h"
#include "debug.h"
#include "undo.h"
#include "cursor.h"
#include "internal_options.h"
#include "buffer.h"
#include "line.h"
#include "mark.h"
#include "mode_pool.h"
#include "action.h"
#include "session.h"
#include "kate/syntaxhighlight.h"
#include "linesearch.h"
#include "folding.h"

/* System */
#include <math.h>

using namespace yzis;

#define STICKY_COL_ENDLINE -1

static const QChar tabChar( '\t' );

static YColor color_null;
static YColor blue( Qt::blue );

#define dbg() yzDebug("YView")
#define err() yzError("YView")

YViewIface::~YViewIface()
{
    // nothing to do
}

/**
 * class YView
 */

static int nextId = 1;

YView::YView(YBuffer *_b, YSession *sess, int cols, int lines)
        : m_drawBuffer(), mainCursor(this), scrollCursor(this), workCursor(this), mVisualCursor(this), keepCursor(this), id(nextId++)
{
    dbg().sprintf("YView( %s, cols=%d, lines=%d )", qp(_b->toString()), cols, lines );
    dbg() << "New View created with UID: " << getId() << endl;
    YASSERT( _b ); YASSERT( sess );
    mSession = sess;
    mBuffer = _b;
    _b->addView( this );
    mLineSearch = new YLineSearch( this );
    mLinesVis = lines;
    mColumnsVis = cols;

    mModePool = new YModePool( this );

    /* start of visual mode */

    mFoldPool = new YZFoldPool( this );

    m_drawBuffer.setCallback( this );

    stickyCol = 0;

    QString line = mBuffer->textline(scrollCursor.bufferY());

    reverseSearch = false;
    viewInformation.l = viewInformation.c1 = viewInformation.c2 = 0;
    viewInformation.percentage = "";
    mPreviousChars = "";
    mLastPreviousChars = "";

    mPaintSelection = new YSelection("PAINT");
    selectionPool = new YSelectionPool();

    drawMode = false;
    rHLnoAttribs = false;
    rHLAttributesLen = 0;

    sCurLineLength = 0;
    rCurLineLength = 0;

    rHLnoAttribs = false;
    rHLAttributesLen = 0;
    listChar = false;
    mFillChar = ' ';

    lineDY = 0;
    tabstop = getLocalIntegerOption("tabstop");
    wrap = getLocalBooleanOption( "wrap" );
    rightleft = getLocalBooleanOption( "rightleft" );
    opt_schema = getLocalIntegerOption( "schema" );
    opt_list = getLocalBooleanOption( "list" );
    opt_listchars = getLocalMapOption( "listchars" );

    abortPaintEvent();

    mModePool->change( YMode::ModeCommand );
}

YView::~YView()
{
    dbg() << "~YView(): Deleting view " << id << endl;
    mModePool->stop();
    mBuffer->saveYzisInfo(this);
    mBuffer->rmView(this); //make my buffer forget about me

    delete selectionPool;
    delete mPaintSelection;
    delete mLineSearch;
    delete mModePool;
    delete mFoldPool;
}

QString YView::toString() const
{
    QString s;
    s.sprintf("View(this=%p id=%d buffer='%s')", this, getId(), qp(myBuffer()->fileName()) );
    return s;
}

void YView::setupKeys()
{
    mModePool->registerModifierKeys();
}

void YView::setVisibleArea(int c, int l, bool refresh)
{
    dbg() << "YView::setVisibleArea(" << c << "," << l << ");" << endl;
    mLinesVis = l;
    mColumnsVis = c;
    if ( refresh )
        recalcScreen();
}

void YView::refreshScreen()
{
    opt_schema = getLocalIntegerOption( "schema" );
    opt_list = getLocalBooleanOption( "list" );
    opt_listchars = getLocalMapOption( "listchars" );
    sendRefreshEvent();
}
void YView::recalcScreen( )
{
    tabstop = getLocalIntegerOption( "tabstop" );
    wrap = getLocalBooleanOption( "wrap" );
    rightleft = getLocalBooleanOption( "rightleft" );

    YCursor old_pos = scrollCursor.buffer();
    scrollCursor.reset();
    if ( wrap ) old_pos.setX( 0 );
    gotoxy( &scrollCursor, old_pos, false );

    old_pos = mainCursor.buffer();
    mainCursor.reset();
    gotoxy( &mainCursor, old_pos );

    sendRefreshEvent();
}

void YView::displayIntro()
{
    mModePool->change( YMode::ModeIntro );
}

YSelectionMap YView::visualSelection() const
{
    return selectionPool->visual()->bufferMap();
}

void YView::reindent(const QPoint pos)
{
    dbg() << "Reindent " << endl;
    QRegExp rx("^(\\t*\\s*\\t*\\s*).*$"); //regexp to get all tabs and spaces
    QString currentLine = mBuffer->textline( pos.y() ).trimmed();
    bool found = false;
    YCursor cur( pos );
    YCursor match = mBuffer->action()->match(this, cur, &found);
    if ( !found ) return ;
    dbg() << "Match found on line " << match.y() << endl;
    QString matchLine = mBuffer->textline( match.y() );
    if ( rx.exactMatch( matchLine ) )
        currentLine.prepend( rx.cap( 1 ) ); //that should have all tabs and spaces from the previous line
    mBuffer->action()->replaceLine( this, YCursor( 0, mainCursor.bufferY() ), currentLine );
    gotoxy( currentLine.length(), mainCursor.bufferY() );
}

/*
* Right now indent just copies the leading whitespace of the current line into the
* new line this isn't smart as it will duplicate extraneous spaces in indentation
* rather than giving consistent depth changes/composition based on user settings.
*/
void YView::indent()
{
    //dbg() << "Entered YView::indent" << endl;
    QString indentMarker = "{"; // Just use open brace for now user defined (BEGIN or whatever) later
    int ypos = mainCursor.bufferY();
    QString currentLine = mBuffer->textline( ypos );
    QRegExp rxLeadingWhiteSpace( "^([ \t]*).*$" );
    if ( !rxLeadingWhiteSpace.exactMatch( currentLine ) ) {
        return ; //Shouldn't happen
    }
    QString indentString = rxLeadingWhiteSpace.cap( 1 );
    if ( mainCursor.bufferX() == currentLine.length() && currentLine.trimmed().endsWith( indentMarker ) ) {
        //dbg() << "Indent marker found" << endl;
        // This should probably be tabstop...
        indentString.append( "\t" );
    }
    //dbg() << "Indent string = \"" << indentString << "\"" << endl;
    mBuffer->action()->insertNewLine( this, mainCursor.buffer() );
    ypos++;
    mBuffer->action()->replaceLine( this, ypos, indentString + mBuffer->textline( ypos ).trimmed() );
    gotoxy( indentString.length(), ypos );
    //dbg() << "Leaving YView::indent" << endl;
}

QString YView::centerLine( const QString& s )
{
    QString spacer = "";
    int nspaces = mColumnsVis > s.length() ? mColumnsVis - s.length() : 0;
    nspaces /= 2;
    spacer.fill( ' ', nspaces );
    spacer.append( s );
    return spacer;
}

void YView::updateCursor()
{
    int lasty = -1;
    viewInformation.percentage = _( "All" );
    int y = mainCursor.bufferY();

    if ( y != lasty ) {
        int nblines = mBuffer->lineCount();
        viewInformation.percentage = QString("%1%").arg( ( y * 100 / ( nblines == 0 ? 1 : nblines )));
        if ( scrollCursor.bufferY() < 1 ) viewInformation.percentage = _( "Top" );
        if ( scrollCursor.bufferY() + mLinesVis >= nblines ) viewInformation.percentage = _( "Bot" );
        if ( (scrollCursor.bufferY() < 1 ) && ( scrollCursor.bufferY() + mLinesVis >= nblines ) ) viewInformation.percentage = _( "All" );
        lasty = y;
    }

    viewInformation.l = y;
    viewInformation.c1 = mainCursor.bufferX();
    viewInformation.c2 = mainCursor.screenX(); // XXX pas du tout, c'est c1 mais en remplacant les tabs par 'tablenght' <-- avec le QRegexp() mais je l'ai perdu <--- English, please :)

    guiSyncViewInfo();
}

void YView::centerViewHorizontally( int column)
{
    // dbg() << "YView::centerViewHorizontally " << column << endl;
    int newcurrentLeft = 0;
    if ( column > mColumnsVis / 2 ) newcurrentLeft = column - mColumnsVis / 2;
    if ( newcurrentLeft != scrollCursor.bufferX() ) {
        // we are not in wrap mode, so buffer == screen
        scrollCursor.setBufferX( newcurrentLeft );
        scrollCursor.setScreenX( newcurrentLeft );
        sendRefreshEvent();
    }
}

void YView::centerViewVertically( int line )
{
    if ( line == -1 )
        line = mainCursor.screenY();
    int newcurrent = 0;
    if ( line > mLinesVis / 2 ) newcurrent = line - mLinesVis / 2;
    alignViewVertically ( newcurrent );
}

void YView::bottomViewVertically( int line )
{
    int newcurrent = 0;
    if ( line >= mLinesVis ) newcurrent = (line - mLinesVis) + 1;
    alignViewVertically( newcurrent );
}

void YView::alignViewBufferVertically( int line )
{
    // dbg() << "YView::alignViewBufferVertically " << line << endl;
    int newcurrent = line;
    int old_dCurrentTop = scrollCursor.screenY();
    if ( newcurrent > 0 ) {
        if ( wrap ) {
            gotodxy( &scrollCursor, scrollCursor.screenX(), newcurrent );
        } else {
            scrollCursor.setBufferY( newcurrent );
            scrollCursor.setScreenY( newcurrent );
        }
    } else {
        scrollCursor.reset();
    }
    if ( old_dCurrentTop == scrollCursor.screenY() ) {
        /* nothing has changed */
        return ;
    } else if ( qAbs(old_dCurrentTop - scrollCursor.screenY()) < mLinesVis ) {
        /* optimization: we can scroll */
        internalScroll( 0, old_dCurrentTop - scrollCursor.screenY() );
    } else {
        sendRefreshEvent();
    }

    // find the last visible line in the buffer
    int lastBufferLineVisible = getCurrentTop() + getLinesVisible() - 1;

    if ( wrap ) {
        YViewCursor temp = scrollCursor;
        gotodxdy( &temp, getCursor().x(), getDrawCurrentTop() + getLinesVisible() - 1 );
        lastBufferLineVisible = temp.bufferY();
    }

    // move cursor if it scrolled off the screen
    if (getCursor().y() < getCurrentTop())
        gotoxy(getCursor().x(), getCurrentTop());
    else if (getCursor().y() > lastBufferLineVisible)
        gotoxy( getCursor().x(), lastBufferLineVisible );

    updateCursor();
}

void YView::alignViewVertically( int line )
{
    // dbg() << "YView::alignViewVertically " << line << endl;
    int newcurrent = line;
    int screenX = scrollCursor.screenX();
    int old_dCurrentTop = scrollCursor.screenY();
    if ( newcurrent > 0 ) {
        if ( wrap ) {
            initGoto( &scrollCursor );
            gotody( newcurrent );
            // rLineHeight > 1 => our new top is in middle of a wrapped line, move new top to next line
            newcurrent = workCursor.bufferY();
            if ( workCursor.lineHeight > 1 )
                ++newcurrent;
            gotoy( newcurrent );
            gotodx( screenX );
            applyGoto( &scrollCursor, false );
        } else {
            scrollCursor.setBufferY( newcurrent );
            scrollCursor.setScreenY( newcurrent );
        }
    } else {
        scrollCursor.reset();
    }

    if ( old_dCurrentTop == scrollCursor.screenY() ) {
        /* nothing has changed */
        return ;
    } else if ( qAbs(old_dCurrentTop - scrollCursor.screenY()) < mLinesVis ) {
        /* optimization: we can scroll */
        internalScroll( 0, old_dCurrentTop - scrollCursor.screenY() );
    } else {
        sendRefreshEvent();
    }
}

/*
 * all the goto-like commands
 */

/* PRIVATE */
void YView::gotodx( int nextx )
{
    YASSERT( nextx >= 0 );
    YZIS_SAFE_MODE {
        if ( nextx < 0 ) nextx = 0;
    }
    int shift = !drawMode && mModePool->current()->isEditMode() && sCurLineLength > 0 ? 0 : 1;
    if ( sCurLineLength == 0 ) nextx = 0;

    // WARNING! we must drawPrevCol _before_ drawNextCol because drawPrevCol doesn't support tab!
    while ( workCursor.screenX() > nextx )
        if ( ! drawPrevCol( ) ) break;
    YViewCursor last = workCursor;
    while ( workCursor.screenX() < nextx && workCursor.bufferX() < sCurLineLength - shift ) {
        last = workCursor;
        drawNextCol( );
    }
    if ( workCursor.screenX() > nextx )
        workCursor = last;
}

void YView::gotox( int nextx, bool forceGoBehindEOL )
{
    YASSERT( nextx >= 0 );
    YZIS_SAFE_MODE {
        if ( nextx < 0 ) nextx = 0;
    }
    int shift = (!drawMode && mModePool->current()->isEditMode() && sCurLineLength > 0) || forceGoBehindEOL ? 1 : 0;
    if ( nextx >= sCurLineLength ) {
        if ( sCurLineLength == 0 ) nextx = 0;
        else nextx = sCurLineLength - 1 + shift;
    }
    while ( workCursor.bufferX() > nextx ) {
        if ( ! wrap || rCurLineLength <= mColumnsVis - shift ) {
            if ( ! drawPrevCol( ) ) break;
        } else {
            if ( ! drawPrevCol( ) ) {
                if ( workCursor.bufferX() >= nextx && workCursor.wrapNextLine ) drawPrevLine( );
                else break;
            }
        }
    }
    bool maywrap = wrap && rCurLineLength + shift > mColumnsVis;
    while ( workCursor.bufferX() < nextx ) {
        if ( !maywrap ) {
            drawNextCol( );
        } else {
            while ( drawNextCol() && workCursor.bufferX() < nextx ) ;
            if ( workCursor.wrapNextLine ) {
                drawNextLine();
                maywrap = rCurLineLength + shift > mColumnsVis;
            } else if ( shift && workCursor.bufferX() == nextx && workCursor.screenX() == mColumnsVis ) {
                workCursor.wrapNextLine = true;
                drawNextLine();
                maywrap = rCurLineLength + shift > mColumnsVis;
            }
        }
    }
}

void YView::gotody( int nexty )
{
    YASSERT( nexty >= 0 );
    YZIS_SAFE_MODE {
        if ( nexty < 0 ) nexty = 0;
    }
    if ( workCursor.bufferY() >= mBuffer->lineCount() ) nexty = qMax( 0, mBuffer->lineCount() - 1 );

    /* some easy case */
    if ( nexty == 0 ) {
        gotoy( 0 );
    } else if ( nexty == scrollCursor.screenY() ) {
        gotoy( scrollCursor.bufferY() );
    } else {
        /** gotody when cursor is > nexty seems buggy, use gotoy way, I'll try to find a better solution */
        bool first = true;
        while ( workCursor.screenY() > nexty ) {
            if ( first && wrap && rCurLineLength > mColumnsVis ) { // move to begin of line
                initDraw( 0, workCursor.bufferY(), 0, workCursor.screenY() - workCursor.lineHeight + 1, drawMode );
                workCursor.lineHeight = workCursor.sLineIncrement = workCursor.bLineIncrement = 1;
                first = false;
            }
            drawPrevLine( );
            if ( wrap && rCurLineLength > mColumnsVis ) {
                /* goto begin of line */
                int wrapLineMinHeight = (int)ceil( rMinCurLineLength / mColumnsVis ) + 1;
                int wrapLineMaxHeight = (int)ceil( rCurLineLength / mColumnsVis ) + 1;
                if ( wrapLineMinHeight == wrapLineMaxHeight ) {
                    workCursor.setScreenY( workCursor.screenY() + 1 - wrapLineMinHeight );
                } else {
                    int cury = workCursor.bufferY();
                    int prevRX = workCursor.screenY();
                    initDraw( 0, cury, 0, 0, drawMode );
                    while ( drawNextCol( ) ) ;
                    while ( workCursor.bufferY() == cury ) {
                        wrapLineMinHeight = workCursor.lineHeight;
                        drawNextLine( );
                        if ( workCursor.bufferY() == cury ) while ( drawNextCol( ) ) ;
                    }
                    initDraw ( 0, cury, 0, prevRX - wrapLineMinHeight + 1, drawMode );
                    workCursor.lineHeight = workCursor.sLineIncrement = workCursor.bLineIncrement = 1;
                }
            }
        }
        while ( workCursor.screenY() < nexty && workCursor.bufferY() < mBuffer->lineCount() - 1 ) {
            if ( wrap && ! workCursor.wrapNextLine && rCurLineLength > mColumnsVis ) // make line wrapping
                while ( drawNextCol( ) ) ;
            drawNextLine( );
            if ( wrap && workCursor.screenY() < nexty && rCurLineLength > mColumnsVis ) // move to end of draw line
                while ( drawNextCol( ) ) ;
        }
    }
}

void YView::gotoy( int nexty )
{
    YASSERT( nexty >= 0 );
    YZIS_SAFE_MODE {
        if ( nexty < 0 ) nexty = 0;
    }
    if ( nexty >= mBuffer->lineCount() ) nexty = qMax( 0, mBuffer->lineCount() - 1 );

    /* some easy case */
    if ( nexty == 0 ) {
        initDraw( 0, 0, 0, 0, drawMode );
        workCursor.lineHeight = workCursor.sLineIncrement = workCursor.bLineIncrement = 1;
    } else if ( nexty == scrollCursor.bufferY() ) {
        bool old_drawMode = drawMode;
        initDraw( ); // XXX
        drawMode = old_drawMode;
        workCursor.lineHeight = workCursor.sLineIncrement = workCursor.bLineIncrement = 1;
    } else if ( nexty != workCursor.bufferY() ) {
        bool first = true;
        while ( workCursor.bufferY() > nexty ) {
            if ( first && wrap && rCurLineLength > mColumnsVis ) { // move to begin of line
                initDraw( 0, workCursor.bufferY(), 0, workCursor.screenY() - workCursor.lineHeight + 1, drawMode );
                workCursor.lineHeight = workCursor.sLineIncrement = workCursor.bLineIncrement = 1;
                first = false;
            }
            drawPrevLine( );
            if ( wrap && rCurLineLength > mColumnsVis ) {
                /* goto begin of line */
                int wrapLineMinHeight = (int)ceil( rMinCurLineLength / mColumnsVis ) + 1;
                int wrapLineMaxHeight = (int)ceil( rCurLineLength / mColumnsVis ) + 1;
                if ( wrapLineMinHeight == wrapLineMaxHeight ) {
                    workCursor.setScreenY( workCursor.screenY() + 1 - wrapLineMinHeight );
                } else {
                    int cury = workCursor.bufferY();
                    int prevRX = workCursor.screenY();
                    initDraw( 0, cury, 0, 0, drawMode );
                    while ( drawNextCol( ) ) ;
                    while ( workCursor.bufferY() == cury ) {
                        wrapLineMinHeight = workCursor.lineHeight;
                        drawNextLine( );
                        if ( workCursor.bufferY() == cury ) while ( drawNextCol( ) ) ;
                    }
                    initDraw ( 0, cury, 0, prevRX - wrapLineMinHeight + 1, drawMode );
                    workCursor.lineHeight = workCursor.sLineIncrement = workCursor.bLineIncrement = 1;
                }
            }
        }
        while ( workCursor.bufferY() < nexty ) {
            if ( wrap && ! workCursor.wrapNextLine && rCurLineLength > mColumnsVis ) // move to end of draw line
                while ( drawNextCol( ) );
            drawNextLine( );
            if ( wrap && workCursor.bufferY() < nexty && rCurLineLength > mColumnsVis ) // move to end of draw line
                while ( drawNextCol( ) ) ;
        }
    }
}

void YView::initGoto( YViewCursor* viewCursor )
{
    initDraw( viewCursor->bufferX(), viewCursor->bufferY(), viewCursor->screenX(), viewCursor->screenY(), false );
    workCursor = *viewCursor;
}

void YView::applyGoto( YViewCursor* viewCursor, bool applyCursor )
{
    *viewCursor = workCursor;

    if ( applyCursor && viewCursor != &mainCursor ) { // do not apply if this isn't the mainCursor
        //  dbg() << "THIS IS NOT THE MAINCURSOR" << endl;
        applyCursor = false;
    } else if ( applyCursor && m_paintAutoCommit > 0 ) {
        sendCursor( *viewCursor );
        applyCursor = false;
    }


    /* dbg() << "applyGoto: "
       << "dColLength=" << dColLength << "; dLineLength=" << dLineLength << "; mLineLength=" << mLineLength
       << "; dWrapNextLine=" << dWrapNextLine << "; dWrapTab=" << dWrapTab << endl;
     dbg() << "mCursor:" << *mCursor << "; dCursor:" << *dCursor << endl; */

    if ( applyCursor ) {

        setPaintAutoCommit( false );

        mModePool->current()->cursorMoved( this );

        if ( !isColumnVisible( mainCursor.screenX(), mainCursor.screenY() ) ) {
            centerViewHorizontally( mainCursor.screenX( ) );
        }
        if ( !isLineVisible( mainCursor.screenY() ) ) {
            if ( mainCursor.screenY() >= mLinesVis + scrollCursor.screenY() )
                bottomViewVertically( mainCursor.screenY() );
            else
                alignViewVertically( mainCursor.screenY() );
        }
        commitPaintEvent();
        updateCursor( );
    }
}


/* goto xdraw, ydraw */
void YView::gotodxdy( QPoint nextpos, bool applyCursor )
{
    gotodxdy( &mainCursor, nextpos, applyCursor );
}
void YView::gotodxdy( YViewCursor* viewCursor, QPoint nextpos, bool applyCursor )
{
    initGoto( viewCursor );
    gotody( nextpos.y() );
    gotodx( nextpos.x() );
    applyGoto( viewCursor, applyCursor );
}

/* goto xdraw, ybuffer */
void YView::gotodxy( int nextx, int nexty, bool applyCursor )
{
    gotodxy( &mainCursor, nextx, nexty, applyCursor );
}
void YView::gotodxy( YViewCursor* viewCursor, int nextx, int nexty, bool applyCursor )
{
    initGoto( viewCursor );
    gotoy( mFoldPool->lineHeadingFold( nexty ) );
    gotodx( nextx );
    applyGoto( viewCursor, applyCursor );
}

/* goto xdraw, ybuffer */
void YView::gotoxdy( int nextx, int nexty, bool applyCursor )
{
    gotoxdy( &mainCursor, nextx, nexty, applyCursor );
}
void YView::gotoxdy( YViewCursor* viewCursor, int nextx, int nexty, bool applyCursor )
{
    initGoto( viewCursor );
    gotody( nexty );
    gotox( nextx );
    applyGoto( viewCursor, applyCursor );
}

/* goto xbuffer, ybuffer */
void YView::gotoxy(const QPoint nextpos, bool applyCursor )
{
    gotoxy( &mainCursor, nextpos, applyCursor );
}
void YView::gotoxy( YViewCursor* viewCursor, const QPoint nextpos, bool applyCursor )
{
    initGoto( viewCursor );
    gotoy( mFoldPool->lineHeadingFold( nextpos.y() ) );
    gotox( nextpos.x(), viewCursor != &mainCursor );
    applyGoto( viewCursor, applyCursor );
}

void YView::gotodxdyAndStick( const QPoint pos )
{
    gotodxdy( &mainCursor, pos, true );
    updateStickyCol( &mainCursor );
}

void YView::gotoxyAndStick( const QPoint pos )
{
    gotoxy( &mainCursor, pos );
    updateStickyCol( &mainCursor );
}

// These all return whether the motion was stopped
bool YView::moveDown( int nb_lines, bool applyCursor )
{
    return moveDown( &mainCursor, nb_lines, applyCursor );
}
bool YView::moveDown( YViewCursor* viewCursor, int nb_lines, bool applyCursor )
{
    int destRequested = mFoldPool->lineAfterFold( viewCursor->bufferY() + nb_lines );
    gotoStickyCol( viewCursor, qMin( destRequested, mBuffer->lineCount() - 1 ), applyCursor );
    return destRequested > (mBuffer->lineCount() - 1);
}
bool YView::moveUp( int nb_lines, bool applyCursor )
{
    return moveUp( &mainCursor, nb_lines, applyCursor );
}
bool YView::moveUp( YViewCursor* viewCursor, int nb_lines, bool applyCursor )
{
    int destRequested = viewCursor->bufferY() - nb_lines;
    gotoStickyCol( viewCursor, qMax( destRequested, 0 ), applyCursor );
    return destRequested < 0;
}

bool YView::moveLeft( int nb_cols, bool wrap, bool applyCursor )
{
    return moveLeft( &mainCursor, nb_cols, wrap, applyCursor );
}

bool YView::moveLeft( YViewCursor* viewCursor, int nb_cols, bool wrap, bool applyCursor )
{
    bool stopped = false;
    int x = int(viewCursor->bufferX());
    int y = viewCursor->bufferY();
    x -= nb_cols;
    if (x < 0) {
        if (wrap) {
            int line_length;
            int diff = -x; // the number of columns we moved too far
            x = 0;
            while (diff > 0 && y >= 1) {
                // go one line up
                line_length = myBuffer()->textline(--y).length();
                dbg() << "line length: " << line_length << endl;
                diff -= line_length + 1;
            }
            // if we moved too far, go back
            if (diff < 0) {
		x -= diff;
		stopped = true;
	    }
	    
        } else {
            x = 0;
	    stopped = true;
	}
	
    }
    gotoxy( viewCursor, x, y);

    if ( applyCursor ) updateStickyCol( viewCursor );

    //return something
    return stopped;
}

bool YView::moveRight( int nb_cols, bool wrap, bool applyCursor )
{
    return moveRight( &mainCursor, nb_cols, wrap, applyCursor );
}

bool YView::moveRight( YViewCursor* viewCursor, int nb_cols, bool wrap, bool applyCursor )
{
    bool stopped = false;
    int x = viewCursor->bufferX();
    int y = viewCursor->bufferY();
    x += nb_cols;
    if (x >= myBuffer()->textline(y).length()) {
        if (wrap) {
            int line_length = myBuffer()->textline(y).length();
            int diff = x - line_length + 1; // the number of columns we moved too far
            x = line_length - 1;
            while (diff > 0 && y < myBuffer()->lineCount() - 1) {
                // go one line down
                line_length = myBuffer()->textline(++y).length();
                x = line_length - 1;
                diff -= line_length + 1;
            }
            // if we moved too far, go back
            if (diff < 0) {
		x += diff;
		stopped = true;
	    }
        } else {
	    stopped = true;
            x = myBuffer()->textline(y).length();
	}
    }
    gotoxy( viewCursor, x, y);

    if ( applyCursor ) updateStickyCol( viewCursor );

    //return something
    return stopped;
}

QString YView::moveToFirstNonBlankOfLine( )
{
    return moveToFirstNonBlankOfLine( &mainCursor );
}

QString YView::moveToFirstNonBlankOfLine( YViewCursor* viewCursor, bool applyCursor )
{
    //execute the code
    gotoxy( viewCursor, mBuffer->firstNonBlankChar(viewCursor->bufferY()) , viewCursor->bufferY(), applyCursor );
    // if ( viewCursor == mainCursor ) UPDATE_STICKY_COL;
    if ( applyCursor )
        updateStickyCol( viewCursor );

    //return something
    return QString();
}

QString YView::moveToStartOfLine( )
{
    return moveToStartOfLine( &mainCursor );
}

QString YView::moveToStartOfLine( YViewCursor* viewCursor, bool applyCursor )
{
    gotoxy(viewCursor, 0 , viewCursor->bufferY(), applyCursor);
    if ( applyCursor )
        updateStickyCol( viewCursor );

    return QString();
}

void YView::gotoLastLine()
{
    gotoLastLine( &mainCursor );
}
void YView::gotoLastLine( YViewCursor* viewCursor, bool applyCursor )
{
    gotoLine( viewCursor, mBuffer->lineCount() - 1, applyCursor );
}
void YView::gotoLine( int line )
{
    gotoLine( &mainCursor, line );
}
void YView::gotoLine( YViewCursor* viewCursor, int line, bool applyCursor )
{
    if ( line >= mBuffer->lineCount() )
        line = mBuffer->lineCount() - 1;

    if ( getLocalBooleanOption("startofline") ) {
        gotoxy( viewCursor, mBuffer->firstNonBlankChar(line), line, applyCursor );
        if ( applyCursor )
            updateStickyCol( viewCursor );
    } else {
        gotoStickyCol( viewCursor, line, applyCursor );
    }
}

QString YView::moveToEndOfLine( )
{
    return moveToEndOfLine( &mainCursor );
}

QString YView::moveToEndOfLine( YViewCursor* viewCursor, bool applyCursor )
{
    gotoxy( viewCursor, mBuffer->textline( viewCursor->bufferY() ).length( ), viewCursor->bufferY(), applyCursor );
    if ( applyCursor )
        stickyCol = STICKY_COL_ENDLINE;

    return QString();
}

void YView::applyStartPosition( const YCursor pos )
{
    if ( pos.y() >= 0 ) {
        //setPaintAutoCommit(false);
        if ( pos.x() >= 0 ) {
            gotoxyAndStick( pos );
        } else {
            gotoLine( pos.y() );
        }
        //centerViewVertically();
        //commitPaintEvent(); XXX keepCursor issue...
    }
}

/**
 * initChanges and applyChanges are called by the buffer to inform the view that there are
 * changes around x,y. Each view have to find what they have to redraw, depending
 * of the wrap option, and of course window size.
 */
void YView::initChanges( QPoint pos)
{
    beginChanges = pos;
    origPos = mainCursor.buffer();
    lineDY = 1;
    if ( wrap && pos.y() < mBuffer->lineCount() ) {
        gotoxy( qMax( 1, mBuffer->getLineLength( pos.y() ) ) - 1, pos.y(), false );
        lineDY = mainCursor.screenY();
    }
    gotoxy( pos, false );
}
void YView::applyChanges( int y )
{
    int dY = mainCursor.screenY() + 1 - mainCursor.lineHeight;
    if ( y != beginChanges.y() ) {
        sendPaintEvent( scrollCursor.screenX(), dY, mColumnsVis, mLinesVis - ( dY - scrollCursor.screenY() ) );
    } else {
        if ( wrap ) {
            gotoxy( qMax( 1, mBuffer->getLineLength( y ) ) - 1, y, false );
            if ( mainCursor.screenY() != lineDY )
                sendPaintEvent( scrollCursor.screenX(), dY, mColumnsVis, mLinesVis - ( dY - scrollCursor.screenY() ) );
            else
                sendPaintEvent( scrollCursor.screenX(), dY, mColumnsVis, 1 + mainCursor.screenY() - dY );
        } else
            sendPaintEvent( scrollCursor.screenX(), dY, mColumnsVis, 1 );
    }
    gotoxy( origPos, false );
}

QString YView::append ()
{
    mModePool->change( YMode::ModeInsert );
    gotoxyAndStick(mainCursor.bufferX() + 1, mainCursor.bufferY() );
    return QString();
}

void YView::commitUndoItem()
{
    mBuffer->undoBuffer()->commitUndoItem(mainCursor.bufferX(), mainCursor.bufferY());
}

/*
 * Drawing engine
 */

bool YView::isColumnVisible( int column, int ) const
{
    return ! (column < scrollCursor.screenX() || column >= (scrollCursor.screenX() + mColumnsVis));
}
bool YView::isLineVisible( int l ) const
{
    return ( ( l >= scrollCursor.screenY() ) && ( l < mLinesVis + scrollCursor.screenY() ) );
}

/* update sCurLine information */
void YView::updateCurLine( )
{
    sCurLineLength = sCurLine.length();
    if ( wrap && ! drawMode ) {
        int nbTabs = sCurLine.count( '\t' );
        rMinCurLineLength = sCurLineLength;
        rCurLineLength = rMinCurLineLength + nbTabs * ( tablength - 1 );
    }
}

int YView::initDrawContents( int clipy )
{
    if ( ! wrap ) {
        initDraw( getCurrentLeft(), clipy, getDrawCurrentLeft(), clipy, true );
    } else {
        int currentY = getDrawCurrentTop();
        initDraw();
        drawMode = true;
        while ( currentY < clipy && drawNextLine() ) {
            while ( drawNextCol() );
            currentY += drawHeight();
        }
        clipy = currentY;
    }
    return clipy;
}

void YView::initDraw( )
{
    initDraw( scrollCursor.bufferX(), scrollCursor.bufferY(), scrollCursor.screenX(), scrollCursor.screenY() );
}

/**
 * initDraw is called before each cursor/draw manipulation.
 */
void YView::initDraw( int sLeft, int sTop, int rLeft, int rTop, bool draw )
{
    sCurrentLeft = sLeft;
    sCurrentTop = sTop;
    rCurrentLeft = rLeft;
    rCurrentTop = rTop;

    workCursor.setBufferX( sCurrentLeft );
    workCursor.setBufferY( sCurrentTop );
    workCursor.setScreenX( rCurrentLeft );
    workCursor.setScreenY( rCurrentTop );

    workCursor.sColIncrement = 1;
    workCursor.bLineIncrement = 0;
    workCursor.sLineIncrement = 0;

    workCursor.lineHeight = 1;

    workCursor.spaceFill = 0;

    adjust = false;

    tablength = tabstop;
    areaModTab = ( tablength - mColumnsVis % tablength ) % tablength;

    workCursor.wrapNextLine = false;
    if ( workCursor.bufferY() < mBuffer->lineCount() ) {
        sCurLine = mBuffer->textline ( workCursor.bufferY() );
        if ( sCurLine.isNull() ) sCurLine = "";
    } else sCurLine = "";

    drawMode = draw;
    updateCurLine( );
}

bool YView::drawPrevLine( )
{
    if ( ! workCursor.wrapNextLine ) {
        if ( workCursor.lineHeight > 1 ) {
            workCursor.sLineIncrement = 0;
            --workCursor.lineHeight;
        } else {
            workCursor.sLineIncrement = 1;
            workCursor.lineHeight = 1;
        }
        workCursor.setBufferX( sCurrentLeft );
        workCursor.setBufferY( mFoldPool->lineHeadingFold( workCursor.bufferY() - workCursor.sLineIncrement ) );
        workCursor.setScreenX( rCurrentLeft );
        if ( workCursor.sLineIncrement == 0 && workCursor.bLineIncrement > 0 ) {
            workCursor.sLineIncrement = 1;
        }
        workCursor.spaceFill = 0;
        workCursor.bLineIncrement = 1;
        workCursor.sColIncrement = 1;
    } else {
        workCursor.setScreenX( mColumnsVis - workCursor.sColIncrement );
        workCursor.spaceFill = workCursor.spaceFill - areaModTab;
        --workCursor.lineHeight;
    }
    workCursor.setScreenY( workCursor.screenY() - workCursor.sLineIncrement );
    workCursor.sLineIncrement = 1;

    if ( workCursor.bufferY() < mBuffer->lineCount() ) {
        if ( ! workCursor.wrapNextLine ) {
            sCurLine = mBuffer->textline( workCursor.bufferY() );
            updateCurLine( );
        }
        if ( rCurrentLeft > 0 && ! workCursor.wrapNextLine ) {
            workCursor.setScreenX( 0 );
            workCursor.setBufferX( 0 );
            gotodx( rCurrentLeft );
        }
        if ( ( workCursor.screenY() - rCurrentTop ) < mLinesVis ) {
            return true;
        }
    } else {
        sCurLine = "";
        sCurLineLength = sCurLine.length();
    }
    workCursor.wrapNextLine = false;
    return false;
}

bool YView::drawNextLine( )
{
    if ( ! workCursor.wrapNextLine ) {
        workCursor.setBufferX( sCurrentLeft );
        workCursor.setBufferY( mFoldPool->lineAfterFold( workCursor.bufferY() + workCursor.bLineIncrement ) );
        workCursor.setScreenX( rCurrentLeft );
        if ( workCursor.sLineIncrement == 0 && workCursor.bLineIncrement > 0 ) {
            // this is need when drawNextCol is called before drawNextLine ( when scrolling )
            workCursor.sLineIncrement = 1;
        }
        workCursor.spaceFill = 0;
        workCursor.bLineIncrement = 1;
        workCursor.lineHeight = 1;
    } else {
        if ( workCursor.wrapTab ) workCursor.setBufferX( workCursor.bufferX() - 1 );
        workCursor.setScreenX( 0 );
        workCursor.spaceFill = ( workCursor.spaceFill + areaModTab ) % tablength;
        ++workCursor.lineHeight;
        if ( workCursor.sLineIncrement == 0 ) {
            workCursor.sLineIncrement = 1;
        }
    }
    workCursor.setScreenY( workCursor.screenY() + workCursor.sLineIncrement );
    workCursor.sLineIncrement = 1;

    if ( workCursor.bufferY() < mBuffer->lineCount() ) {
        YLine* yl = drawMode ? mBuffer->yzline( workCursor.bufferY(), false ) : NULL;
        if ( ! workCursor.wrapNextLine ) {
            sCurLine = drawMode ? yl->data() : mBuffer->textline( workCursor.bufferY() );
            updateCurLine( );
        }
        if ( rCurrentLeft > 0 && ! workCursor.wrapNextLine ) {
            workCursor.setBufferX( 0 );
            workCursor.setScreenX( 0 );
            adjust = true;
            gotodx( rCurrentLeft );
            adjust = false;
            if ( drawMode ) {
                if ( scrollCursor.bufferX() > 0 )
                    workCursor.spaceFill = ( tablength - scrollCursor.bufferX() % tablength ) % tablength;
                if ( workCursor.screenX() > rCurrentLeft ) {
                    workCursor.setBufferX( workCursor.bufferX() - 1 );
                    workCursor.setScreenX( rCurrentLeft );
                }
            }
        }
        if ( drawMode && ( workCursor.screenY() - rCurrentTop ) < mLinesVis ) {
            m_lineFiller = ' ';
            m_lineMarker = ' ';
            if ( mFoldPool->isHead( workCursor.bufferY() ) ) {
                m_lineFiller = '-';
                m_lineMarker = '+';
            }
            rHLa = NULL;
            if ( yl->length() != 0 )
                rHLa = yl->attributes();
            rHLnoAttribs = !rHLa;
            rHLa = rHLa + workCursor.bufferX() - 1;
            rHLAttributes = 0L;
            YzisHighlighting * highlight = mBuffer->highlight();
            if ( highlight )
                rHLAttributes = highlight->attributes( opt_schema )->data( );
            rHLAttributesLen = rHLAttributes ? highlight->attributes( opt_schema )->size() : 0;
            return true;
        }
    } else {
        sCurLine = "";
        sCurLineLength = sCurLine.length();
    }
    workCursor.wrapNextLine = false;
    return false;
}

bool YView::drawPrevCol( )
{
    workCursor.wrapNextLine = false;
    int shift = !drawMode && mModePool->current()->isEditMode() && sCurLineLength > 0 ? 1 : 0;
    if ( workCursor.bufferX() >= workCursor.bColIncrement ) {
        int curx = workCursor.bufferX() - 1;
        workCursor.setBufferX( curx );
        lastChar = sCurLine.at( curx );
        if ( lastChar != tabChar ) {
            workCursor.sColIncrement = 1;
            if ( workCursor.screenX() >= workCursor.sColIncrement )
                workCursor.setScreenX( workCursor.screenX() - workCursor.sColIncrement );
            else
                workCursor.wrapNextLine = ( wrap && rCurLineLength > mColumnsVis - shift && workCursor.screenX() == 0 && workCursor.bufferX() > 0 );
            workCursor.bLineIncrement = workCursor.wrapNextLine ? 0 : 1;
        } else {
            /* go back to begin of line */
            initDraw( 0, workCursor.bufferY(), 0, workCursor.screenY() - workCursor.lineHeight + 1, drawMode );
            return false;
        }
    }
    return ! workCursor.wrapNextLine;
}

bool YView::drawNextCol( )
{
    bool ret = false;

    int curx = workCursor.bufferX();
    bool lastCharWasTab = workCursor.lastCharWasTab;

    int nextLength = ( drawMode ? 0 : 1 );

    workCursor.sColIncrement = 1;
    workCursor.wrapNextLine = false;
    workCursor.lastCharWasTab = false;

    if ( curx < sCurLineLength ) {
        int lenToTest;
        lastChar = sCurLine.at( curx );
        mFillChar = ' ';
        if ( lastChar != tabChar ) {
            listChar = drawMode && opt_list && lastChar == ' ';
            if ( listChar ) {
                if ( stringHasOnlySpaces(sCurLine.mid(curx) ) && opt_listchars[ "trail" ].length() > 0 ) {
                    lastChar = opt_listchars["trail"][0];
                } else if ( opt_listchars["space"].length() > 0 ) {
                    lastChar = opt_listchars["space"][0];
                }
            }
            workCursor.sColIncrement = 1;
            lenToTest = workCursor.sColIncrement;
        } else {
            workCursor.lastCharWasTab = true;
            lastChar = ' ';
            listChar = drawMode && opt_list;
            if ( listChar ) {
                if ( opt_listchars["tab"].length() > 0 ) {
                    lastChar = opt_listchars["tab"][0];
                    if ( opt_listchars["tab"].length() > 1 )
                        mFillChar = opt_listchars["tab"][1];
                    if ( workCursor.wrapTab )
                        lastChar = mFillChar;
                }
            }
            if ( workCursor.screenX( ) == scrollCursor.bufferX() )
                workCursor.sColIncrement = ( workCursor.spaceFill ? workCursor.spaceFill : tablength );
            else {
                // calculate tab position
                int mySpaceFill = ( scrollCursor.bufferX() == 0 ? workCursor.spaceFill : 0 ); // do not care about rSpaceFill if we arent't in wrapped mode
                if ( workCursor.screenX() >= mySpaceFill )
                    workCursor.sColIncrement = ( ( workCursor.screenX() - mySpaceFill ) / tablength + 1 ) * tablength + mySpaceFill - workCursor.screenX();
                else
                    workCursor.sColIncrement = mySpaceFill + 1 - workCursor.screenX();
            }
            if ( drawMode ) lenToTest = 1;
            else lenToTest = workCursor.sColIncrement;
        }

        // will our new char appear in the area ?
        ret = adjust || workCursor.screenX() + lenToTest - scrollCursor.screenX() <= mColumnsVis - nextLength;

        if ( ret || ! drawMode ) {
            // moving cursors
            workCursor.setScreenX( workCursor.screenX() + workCursor.sColIncrement );
            workCursor.setBufferX( workCursor.bufferX() + workCursor.bColIncrement );
            // update HL
            if ( drawMode ) rHLa += workCursor.bColIncrement;
        }
    } else if ( sCurLineLength == 0 && drawMode && curx == 0 ) { // empty line
        ret = true;
        lastChar = ' ';
        workCursor.setScreenX( 1 );
        workCursor.setBufferX( 1 );
    }

    if ( wrap ) {
        // screen pos
        int sx = workCursor.screenX() + nextLength + ( (ret || !drawMode) ? 0 : workCursor.sColIncrement );
        // buff pos
        int bx = curx + ( drawMode ? 0 : workCursor.bColIncrement );
        workCursor.wrapNextLine = sx > mColumnsVis;
        if ( bx == sCurLineLength ) { // wrap a tab at EOL
            workCursor.wrapNextLine &= ( drawMode ? lastCharWasTab : workCursor.lastCharWasTab );
        } else {
            workCursor.wrapNextLine &= bx < sCurLineLength;
        }
    }

    // only remember of case where wrapNextLine is true ( => we will wrap a tab next drawNextCol )
    workCursor.lastCharWasTab &= workCursor.wrapNextLine;

    // wrapNextLine is true, we are out of area ( ret is false ), last char was a tab => we are wrapping a tab
    workCursor.wrapTab = false;
    if ( workCursor.wrapNextLine ) {
        if ( drawMode ) {
            workCursor.wrapTab = ! ret && lastCharWasTab;
        } else {
            workCursor.wrapTab = workCursor.lastCharWasTab && workCursor.screenX() > mColumnsVis;
        }
    }
    // do not increment line buffer if we are wrapping a line
    workCursor.bLineIncrement = workCursor.wrapNextLine ? 0 : 1;

    /* if ( !drawMode && (workCursor.bufferY() == 12 || workCursor.bufferY() == 13 ) ) {
      workCursor.debug();
      dbg() << ret << endl;
     }
    */ 
    return ret;
}

const QChar& YView::fillChar() const
{
    return mFillChar;
}
const QChar& YView::drawChar() const
{
    return lastChar;
}
const QChar& YView::drawLineFiller() const
{
    return m_lineFiller;
}
const QChar& YView::drawLineMarker() const
{
    return m_lineMarker;
}
int YView::drawLength() const
{
    return workCursor.sColIncrement;
}
int YView::drawHeight() const
{
    return workCursor.sLineIncrement;
}
int YView::lineIncrement() const
{
    return workCursor.bLineIncrement;
}
int YView::lineHeight() const
{
    return workCursor.lineHeight;
}

const YColor& YView::drawColor ( int col, int line ) const
{
    YLine *yl = mBuffer->yzline( line );
    YzisHighlighting * highlight = mBuffer->highlight();
    const uchar* hl = NULL;
    YzisAttribute *at = NULL;

    if ( yl->length() != 0 && highlight ) {
        hl = yl->attributes(); //attributes of this line
        hl += col; // -1 ? //move pointer to the correct column
        int len = hl ? highlight->attributes( 0 )->size() : 0 ; //length of attributes
        int schema = getLocalIntegerOption("schema");
        YzisAttribute *list = highlight->attributes( schema )->data( ); //attributes defined by the syntax highlighting document
        at = ( ( *hl ) >= len ) ? &list[ 0 ] : &list[*hl]; //attributes pointed by line's attribute for current column
    }
    if ( opt_list && ( yl->data().at(col) == ' ' || yl->data().at(col) == tabChar ) )
        return blue;
    if ( at ) return at->textColor(); //textcolor :)
    return color_null;
}

const YColor& YView::drawColor()
{
    curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

    if ( listChar ) return blue; //XXX make custom
    else if ( curAt ) return curAt->textColor();
    else return color_null;
}

const YColor& YView::drawSelColor()
{
    curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

    if ( listChar ) return color_null; //XXX make custom
    else if ( curAt ) return (*curAt).selectedTextColor();
    else return color_null;
}

const YColor& YView::drawBgColor()
{
    curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

    if ( listChar ) return color_null; //XXX make custom
    else if ( curAt ) return (*curAt).bgColor();
    else return color_null;
}

const YColor& YView::drawBgSelColor()
{
    curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

    if ( listChar ) return color_null; //XXX make custom
    else if ( curAt ) return (*curAt).selectedBGColor();
    else return color_null;
}

bool YView::drawBold()
{
    curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
    if ( curAt )
        return (*curAt).bold();
    return false;
}

bool YView::drawItalic()
{
    curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
    if ( curAt )
        return (*curAt).italic();
    return false;
}

bool YView::drawUnderline()
{
    curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
    if ( curAt )
        return (*curAt).underline();
    return false;
}

bool YView::drawOverline()
{
    curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
    if ( curAt )
        return (*curAt).overline();
    return false;
}

bool YView::drawStrikeOutLine()
{
    curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
    if ( curAt )
        return (*curAt).strikeOut();
    return false;
}

const YColor& YView::drawOutline()
{
    curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

    if ( listChar ) return color_null; //XXX make custom
    else if ( curAt ) return (*curAt).outline();
    else return color_null;
}

int YView::drawLineNumber() const
{
    return workCursor.bufferY( ) + 1;
}

int YView::drawTotalHeight()
{
    int totalHeight = 0;
    int nb = mBuffer->lineCount();
    if ( nb > 0 ) {
        YViewCursor cursor = viewCursor();
        int x = mBuffer->textline( nb - 1 ).length();
        if ( x > 0 ) --x;
        gotoxy( &cursor, x, nb - 1 );
        totalHeight = cursor.screenY() + 1;
    }
    return totalHeight;
}

void YView::printToFile( const QString& /*path*/ )
{
#if 0
    if ( YSession::getStringOption("printer") != "pslib" ) {
        if ( getenv( "DISPLAY" ) ) {
            YZQtPrinter qtprinter( this );
            qtprinter.printToFile( path );
            qtprinter.run( );
        } else {
            YSession::self()->guiPopupMessage( _("To use the Qt printer, you need to have an X11 DISPLAY set and running, you should try pslib in console mode") );
        }
        return ;
    }

#ifdef HAVE_LIBPS
    YZPrinter printer( this );
    printer.printToFile( path );
    printer.run( );
#endif
#endif
}

void YView::undo( int count )
{
    for ( int i = 0 ; i < count ; i++ )
        mBuffer->undoBuffer()->undo( this );
}

void YView::redo( int count )
{
    for ( int i = 0 ; i < count ; i++ )
        mBuffer->undoBuffer()->redo( this );
}


QString YView::getLocalOptionKey() const
{
    return mBuffer->fileName() + "-view-" + QString::number(getId());
}
YOptionValue* YView::getLocalOption( const QString& option ) const
{
    if ( YSession::self()->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) ) //find the local one ?
        return YSession::self()->getOptions()->getOption( getLocalOptionKey() + "\\" + option );
    else
        return YSession::self()->getOptions()->getOption( "Global\\" + option );
}
int YView::getLocalIntegerOption( const QString& option ) const
{
    if ( YSession::self()->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) ) //find the local one ?
        return YSession::self()->getOptions()->readIntegerOption( getLocalOptionKey() + "\\" + option );
    else
        return YSession::self()->getOptions()->readIntegerOption( "Global\\" + option ); // else give the global default if any
}
bool YView::getLocalBooleanOption( const QString& option ) const
{
    if ( YSession::self()->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) ) //find the local one ?
        return YSession::self()->getOptions()->readBooleanOption( getLocalOptionKey() + "\\" + option );
    else
        return YSession::self()->getOptions()->readBooleanOption( "Global\\" + option );
}
QString YView::getLocalStringOption( const QString& option ) const
{
    if ( YSession::self()->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) ) //find the local one ?
        return YSession::self()->getOptions()->readStringOption( getLocalOptionKey() + "\\" + option );
    else
        return YSession::self()->getOptions()->readStringOption( "Global\\" + option );
}
QStringList YView::getLocalListOption( const QString& option ) const
{
    if ( YSession::self()->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) ) //find the local one ?
        return YSession::self()->getOptions()->readListOption( getLocalOptionKey() + "\\" + option );
    else
        return YSession::self()->getOptions()->readListOption( "Global\\" + option );
}
MapOption YView::getLocalMapOption( const QString& option ) const
{
    if ( YSession::self()->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) ) //find the local one ?
        return YSession::self()->getOptions()->readMapOption( getLocalOptionKey() + "\\" + option );
    else
        return YSession::self()->getOptions()->readMapOption( "Global\\" + option );
}

void YView::gotoStickyCol( int Y )
{
    gotoStickyCol( &mainCursor, Y, true );
}

void YView::gotoStickyCol( YViewCursor* viewCursor, int Y, bool applyCursor )
{
    if ( stickyCol == STICKY_COL_ENDLINE )
        gotoxy( viewCursor, mBuffer->textline( Y ).length(), Y, applyCursor );
    else {
        int col = stickyCol % mColumnsVis;
        int deltaY = stickyCol / mColumnsVis;
        if ( deltaY == 0 ) {
            gotodxy( viewCursor, col, Y, applyCursor );
        } else {
            int lineLength = mBuffer->textline( Y ).length();
            gotoxy( viewCursor, 0, Y, false );
            int startDY = viewCursor->screenY();
            gotoxy( viewCursor, lineLength, Y, false );
            int endDY = viewCursor->screenY();
            if ( startDY + deltaY > endDY ) {
                gotoxy( viewCursor, lineLength, Y, applyCursor );
            } else {
                gotodxdy( viewCursor, col, startDY + deltaY, applyCursor );
            }
        }
    }
}

QString YView::getCharBelow( int delta )
{
    YViewCursor vc = viewCursor();
    int Y = vc.bufferY();
    if ( delta < 0 && Y >= -delta || delta >= 0 && ( Y + delta ) < mBuffer->lineCount() )
        Y += delta;
    else
        return QString();

    QString ret;
    int dx = vc.screenX();
    int old_stickyCol = stickyCol;
    updateStickyCol( &vc );
    gotoStickyCol( &vc, Y, false );
    if ( vc.screenX() >= dx ) {
        int x = vc.bufferX();
        if ( vc.screenX() > dx && x > 0 ) // tab
            --x;
        QString l = mBuffer->textline( Y );
        if ( x < l.length() )
            ret = l.at( x );
    }

    // restore stickyCol
    stickyCol = old_stickyCol;
    return ret;
}

void YView::updateStickyCol( )
{
    updateStickyCol( &mainCursor );
}
void YView::updateStickyCol( YViewCursor* viewCursor )
{
    stickyCol = ( viewCursor->lineHeight - 1 ) * mColumnsVis + viewCursor->screenX();
}

void YView::commitNextUndo()
{
    mBuffer->undoBuffer()->commitUndoItem( mainCursor.bufferX(), mainCursor.bufferY() );
}
const YCursor YView::getCursor() const
{
    return mainCursor.screen();
}
const YCursor YView::getBufferCursor() const
{
    return mainCursor.buffer();
}
YCursor YView::getRelativeScreenCursor() const
{
    return (QPoint)mainCursor.screen() - scrollCursor.screen();
}
YCursor YView::getScreenPosition() const
{
    return scrollCursor.screen();
}

void YView::recordMacro( const QList<QChar> &regs )
{
    mRegs = regs;
    for ( int ab = 0 ; ab < mRegs.size(); ++ab )
        YSession::self()->setRegister( mRegs.at(ab), QStringList());
}

void YView::stopRecordMacro()
{
    for ( int ab = 0 ; ab < mRegs.size(); ++ab ) {
        QStringList list;
        QString ne = YSession::self()->getRegister(mRegs.at(ab))[0];
        list << ne.mid( 0, ne.length() - 1 ); //remove the last 'q' which was recorded ;)
        YSession::self()->setRegister( mRegs.at(ab), list);
    }
    mRegs = QList<QChar>();
}

YSelection YView::clipSelection( const YSelection& sel ) const
{
    YCursor bottomRight = scrollCursor.screen() + YCursor(getColumnsVisible() - 1, getLinesVisible() - 1);
    return sel.clip( YInterval(scrollCursor.screen(), bottomRight) );
}

void YView::setPaintAutoCommit( bool enable )
{
    if ( enable ) {
        m_paintAutoCommit = 0;
    } else {
        ++m_paintAutoCommit;
    }
}

void YView::commitPaintEvent()
{
    if ( m_paintAutoCommit == 0 ) return ;
    if ( --m_paintAutoCommit == 0 ) {
        if ( keepCursor.valid() ) {
            mainCursor = keepCursor;
            keepCursor.invalidate();
            applyGoto( &mainCursor );
        }
        if ( ! mPaintSelection->isEmpty() ) {
            guiNotifyContentChanged( clipSelection(*mPaintSelection) );
        }
        abortPaintEvent();
    }
}
void YView::abortPaintEvent()
{
    keepCursor.invalidate();
    mPaintSelection->clear();
    setPaintAutoCommit();
}

void YView::sendCursor( YViewCursor cursor )
{
    keepCursor = cursor;
}
void YView::sendPaintEvent( const YCursor from, const YCursor to )
{
    m_paintAll = false;
    setPaintAutoCommit( false );
    mPaintSelection->addInterval( YInterval( from, to ) );
    commitPaintEvent();
}
void YView::sendPaintEvent( int curx, int cury, int curw, int curh )
{
    if ( curh == 0 ) {
        dbg() << "Warning: YView::sendPaintEvent with height = 0" << endl;
        return ;
    }
    sendPaintEvent( YCursor(curx, cury), YCursor(curx + curw, cury + curh - 1) );
}
void YView::sendPaintEvent( YSelectionMap map, bool isBufferMap )
{
    int size = map.size();
    int i;
    if ( isBufferMap && wrap ) { // we must convert bufferMap to screenMap
        YViewCursor vCursor = viewCursor();
        for ( i = 0; i < size; i++ ) {
            gotoxy( &vCursor, map[ i ].fromPos().x(), map[ i ].fromPos().y() );
            map[ i ].setFromPos( vCursor.screen() );
            gotoxy( &vCursor, map[ i ].toPos().x(), map[ i ].toPos().y() );
            map[ i ].setToPos( vCursor.screen() );
        }
    }
    setPaintAutoCommit( false );
    mPaintSelection->addMap( map );
    commitPaintEvent();
}
void YView::sendBufferPaintEvent( int line, int n )
{
    YViewCursor vCursor = viewCursor();
    if ( wrap ) {
        gotoxy( &vCursor, 0, line );
        line = vCursor.screenY();
    }
    if ( isLineVisible( line ) ) {
        if ( wrap ) {
            gotoxy( &vCursor, 0, line + n );
            n = vCursor.screenY() - line;
        }
        sendPaintEvent( getDrawCurrentLeft(), line, getColumnsVisible(), n );
    }
}

void YView::sendRefreshEvent( )
{
    mPaintSelection->clear();
    m_paintAll = true;
    sendPaintEvent( getDrawCurrentLeft(), getDrawCurrentTop(), getColumnsVisible(), getLinesVisible() );
}

void YView::removePaintEvent( const YCursor from, const YCursor to )
{
    m_paintAll = false;
    mPaintSelection->delInterval( YInterval( from, to ) );
}

bool YView::stringHasOnlySpaces ( const QString& what )
{
    for (int i = 0 ; i < what.length(); i++)
        if ( !what.at(i).isSpace() ) {
            return false;
        }
    return true;
}

QString YView::modeString() const
{
    QString ret = mModePool->current()->toString();
    if ( isRecording() )
        ret += _(" { Recording }");
    return ret;
}

void YView::saveInputBuffer()
{
    // We don't need to remember ENTER or ESC or CTRL-C
    if ( mPreviousChars == "<ENTER>"
            || mPreviousChars == "<ESC>"
            || mPreviousChars == "<CTRL>c" ) {
        return ;
    }

    // If we are repeating the command don't overwrite
    if ( mPreviousChars != "." ) {
        mLastPreviousChars = mPreviousChars;
    }
}

const int YView::getId() const
{
    return id;
}

QString YView::getLineStatusString() const
{
    QString status;

    if (viewInformation.c1 != viewInformation.c2) {
        status = QString("%1,%2-%3 (%4)")
                 .arg(viewInformation.l + 1 )
                 .arg( viewInformation.c1 + 1 )
                 .arg( viewInformation.c2 + 1 )
                 .arg( viewInformation.percentage);
    } else {
        status = QString("%1,%2 (%3)")
                 .arg(viewInformation.l + 1 )
                 .arg( viewInformation.c1 + 1 )
                 .arg( viewInformation.percentage );
    }

    return status;
}

void YView::internalScroll( int dx, int dy )
{
    m_drawBuffer.Scroll( dx, dy );
    guiScroll( dx, dy );
}

/**
 * default implementation for guiPaintEvent
 */
void YView::guiPaintEvent( const YSelection& drawMap )
{
    if ( drawMap.isEmpty() )
        return ;
    //dbg() << "YView::guiPaintEvent" << drawMap;

    bool number = getLocalBooleanOption( "number" );
    if ( number ) {
        guiDrawSetMaxLineNumber( myBuffer()->lineCount() );
    }
    if ( m_paintAll ) {
        /* entire screen has been updated already */
        return ;
    }

    /* to calculate relative position */
    int shiftY = getDrawCurrentTop();
    int shiftX = getDrawCurrentLeft();
    int maxX = shiftX + getColumnsVisible() - 1;

    YSelectionMap map = drawMap.map();
    int size = map.size();

    int fromY = map[ 0 ].fromPos().y(); /* first line */
    int toY = map[ size - 1 ].toPos().y(); /* last line */

    /* where the draw begins */
    int curY = initDrawContents( fromY );
    int curX = 0;

    /* inform the view we want to paint from line <fromY> to <toY> */
    guiPreparePaintEvent( curY - shiftY, toY - shiftY );

    int mapIdx = 0; /* first interval */

    int fX = map[ mapIdx ].fromPos().x(); /* first col of interval */
    int fY = map[ mapIdx ].fromPos().y(); /* first line of interval */
    int tX = map[ mapIdx ].toPos().x(); /* last col of interval */
    int tY = map[ mapIdx ].toPos().y(); /* last line of interval */

    bool drawIt; /* if we are inside the interval */

    bool drawLine; /* if we have to paint a part of line */
    bool drawEntireLine; /* if we have to paint the entire line */

    bool drawStartAfterBOL; /* if we don't have to draw from the begin of line */
    bool drawStopBeforeEOL; /* if we don't have to draw until the end of line */

    bool clearToEOL; /* if we have to clear the rest of line */

    bool interval_changed = true;

    /* set selection layouts */
    m_drawBuffer.setSelectionLayout( YSelectionPool::Search, *getSelectionPool()->search() - scrollCursor.screen() ); //XXX: search map is buffer only
    m_drawBuffer.setSelectionLayout( YSelectionPool::Visual, getSelectionPool()->visual()->screen() - scrollCursor.screen() );

    while ( curY <= toY && drawNextLine() ) {
        curX = shiftX;

        while ( tY < curY ) { /* next interval */
            ++mapIdx;
            fX = map[ mapIdx ].fromPos().x();
            fY = map[ mapIdx ].fromPos().y();
            tX = map[ mapIdx ].toPos().x();
            tY = map[ mapIdx ].toPos().y();
            interval_changed = true;
        }
        //yzDebug("painting") << curX << "," << curY << ":painting interval " << map[ mapIdx ]
        //      << " (changed=" << interval_changed << ")" << endl;
        if ( interval_changed ) {
            m_drawBuffer.replace( map[ mapIdx ] - scrollCursor.screen() );
            interval_changed = false;
        }

        drawStartAfterBOL = ( curY == fY && fX > shiftX );
        drawStopBeforeEOL = ( curY == tY && tX < maxX );

        drawLine = fY <= curY; // curY <= tY always true */
        drawIt = drawLine && !drawStartAfterBOL;
        drawEntireLine = drawIt && !drawStopBeforeEOL;

        clearToEOL = drawEntireLine || drawIt && curY != tY;

        if ( drawLine && number ) {
            guiDrawSetLineNumber( curY - shiftY, drawLineNumber(), lineHeight() - 1 );
        }

        if ( drawIt && curY > fY ) {
            m_drawBuffer.newline( curY - shiftY );
        }
        while ( drawNextCol() ) {
            if ( !drawEntireLine ) { /* we have to care of starting/stopping to draw on that line */
                if ( !drawIt && curY == fY ) { // start drawing 
                    drawIt = ( curX == fX );
                    if ( curX < fX && curX + drawLength() >= fX ) { // Next char will overlap beginning of redraw
                        m_drawBuffer.replace( YInterval(YCursor(curX, curY), YCursor(fX-1, curY)) );
                        drawIt = true;
                    }
                    clearToEOL = drawIt && curY != tY;
                }
                if ( drawIt && curY == tY ) { // stop drawing ?
                    if ( curX > tX+1 ) // Char just added actually overlaps existing cells
                        m_drawBuffer.replace( YInterval(YCursor(curX, curY),
                                                        YCursor(curX + (curX - tX - 2), curY)) );
                    drawIt = !( curX > tX );
                    if ( ! drawIt ) {
                        ++mapIdx;
                        if ( mapIdx != size ) {
                            fX = map[ mapIdx ].fromPos().x();
                            fY = map[ mapIdx ].fromPos().y();
                            tX = map[ mapIdx ].toPos().x();
                            tY = map[ mapIdx ].toPos().y();
                            m_drawBuffer.replace( map[ mapIdx ] - scrollCursor.screen() );
                        } else {
                            fX = fY = tX = tY = 0;
                        }
                    }
                }
            }
            if ( drawIt ) {
                QString disp = QString( drawChar() );
                disp = disp.leftJustified( drawLength(), fillChar() );

                m_drawBuffer.setColor( drawColor() );

                m_drawBuffer.push( disp );
            }
            curX += drawLength();
        }
        if ( clearToEOL ) {
            guiDrawClearToEOL( QPoint (curX - shiftX, curY - shiftY), drawLineFiller() );
        }
        curY += drawHeight();
    }

    /* out of file lines (~) */
    int fh = shiftY + getLinesVisible();
    toY = qMin( toY, fh - 1 );
    m_drawBuffer.setColor( Qt::cyan );
    for ( ; curY <= toY; ++curY ) {
        m_drawBuffer.newline( curY - shiftY );
        if ( number ) guiDrawSetLineNumber( curY - shiftY, 0, 0 );
        m_drawBuffer.push( "~" );
        guiDrawClearToEOL( QPoint(1, curY - shiftY), ' ' );
    }

    m_drawBuffer.flush();

    // dbg() << "after drawing: " << endl << m_drawBuffer << "--------" << endl;

    guiEndPaintEvent();
}

