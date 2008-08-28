/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>.
*  Copyright (C) 2003-2008 Loic Pauleve <panard@inzenet.org>
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
#include "drawbuffer.h"

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
    // nothing to do but compiler will complain if ~YZViewIface() is pure
    // virtual.
}

/**
 * class YView
 */

static int nextId = 1;

YView::YView(YBuffer *_b, YSession *sess, int cols, int lines)
		:
	mDrawBuffer(cols,lines),
	mPreviousChars(""),mLastPreviousChars(""),
	mainCursor(), workCursor(),
	mSelectionPool(),
	mPaintSelection(),
	keepCursor(),
	id(nextId++)
{
    dbg().SPrintf("YView( %s, cols=%d, lines=%d )", qp(_b->toString()), cols, lines );
    dbg() << "New View created with UID: " << getId() << endl;
    YASSERT( _b ); YASSERT( sess );
    mSession = sess;
    mBuffer = _b;
    _b->addView( this );
    mLineSearch = new YLineSearch( this );

    mModePool = new YModePool( this );

    /* start of visual mode */

    mFoldPool = new YZFoldPool( this );

    stickyCol = 0;

    reverseSearch = false;
    mPreviousChars.clear();

	updateInternalAttributes();

    abortPaintEvent();
}

YView::~YView()
{
    dbg() << "~YView(): Deleting view " << id << endl;
    mModePool->stop();
    mBuffer->saveYzisInfo(this);
    mBuffer->rmView(this); //make my buffer forget about me
    if (mBuffer->views().isEmpty()) {
        // last view deleted, delete the buffer
        YSession::self()->deleteBuffer( mBuffer );
    }

    delete mLineSearch;
    delete mModePool;
    delete mFoldPool;
}

QString YView::toString() const
{
    QString s;
    s.sprintf("View(this=%p id=%d buffer='%s')", this, getId(), qp(myBuffer()->fileNameShort()) );
    return s;
}

void YView::setupKeys()
{
    mModePool->registerModifierKeys();
}

void YView::setVisibleArea( int c, int l )
{
    dbg() << "setVisibleArea(" << c << "," << l << ")" << endl;
	if ( c != mDrawBuffer.screenWidth() || l != mDrawBuffer.screenHeight() ) {
		mDrawBuffer.setScreenSize(c, l);
        recalcScreen();
	}
}

void YView::updateInternalAttributes()
{
    tabstop = getLocalIntegerOption("tabstop");
    wrap = getLocalBooleanOption( "wrap" );
    rightleft = getLocalBooleanOption( "rightleft" );
    opt_list = getLocalBooleanOption( "list" );
    opt_listchars = getLocalMapOption( "listchars" );

    opt_schema = getLocalIntegerOption( "schema" );
	YzisHighlighting* highlight = mBuffer->highlight();
	if ( highlight ) {
		mHighlightAttributes = highlight->attributes(opt_schema)->data();
	} else {
		mHighlightAttributes = NULL;
	}
}

void YView::refreshScreen()
{
	updateInternalAttributes();
    sendRefreshEvent();
}
void YView::recalcScreen( )
{
	dbg() << "recalcScreen" << endl;
	updateInternalAttributes();

    YCursor old_pos = mainCursor.buffer();
    mainCursor.reset();

	setPaintAutoCommit(false);

	updateBufferInterval(YInterval(YCursor(0,0), YBound(YCursor(0, mBuffer->lineCount()), true)));
	sendRefreshEvent();

    gotoxy( &mainCursor, old_pos );

	guiSetup();

	commitPaintEvent();
}

void YView::displayIntro()
{
    mModePool->change( YMode::ModeIntro );
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
    int nspaces = mDrawBuffer.screenWidth() > s.length() ? mDrawBuffer.screenWidth() - s.length() : 0;
    nspaces /= 2;
    spacer.fill( ' ', nspaces );
    spacer.append( s );
    return spacer;
}

void YView::updateCursor()
{
    int lasty = -1;
    QString percentage;
    QString lineinfo;

    int y = mainCursor.bufferY();

    if (y != lasty) {
        int nblines = mBuffer->lineCount();
        if (getCurrentTop() < 1)
            if ((getCurrentTop() + getLinesVisible()) >= nblines)
                    percentage = _("All");
            else
                    percentage =  _("Top");

        else if ((getCurrentTop() + getLinesVisible()) >= nblines)
            percentage = _("Bot");

        else
            if(y < 0 || y > nblines)
                err() << HERE() << "Percentage out of range" << endl;
            else
                percentage.setNum((int)(y * 100 / (nblines == 0 ? 1 : nblines)));

    } else {
        percentage = _("All");
    }

    if (guiStatusBar())
        guiStatusBar()->setLineInfo(y + 1, viewCursor().bufferX() + 1,
                                    viewCursor().screenX() + 1, percentage);
    guiUpdateCursorPosition();
}

void YView::updateMode()
{
    QString mode;

    mode = currentMode()->toString();
    if (isRecording())
        mode += _(" { Recording }");

    if (guiStatusBar())
        guiStatusBar()->setMode(mode);
    guiUpdateMode();
}

void YView::updateFileName()
{
    QString filename = myBuffer()->fileName();
    if (guiStatusBar())
        guiStatusBar()->setFileName(filename);
    guiUpdateFileName();
}

void YView::updateFileInfo()
{
    if (guiStatusBar())
        guiStatusBar()->setFileInfo(myBuffer()->fileIsNew(), myBuffer()->fileIsModified());
    guiUpdateFileInfo();
}

void YView::displayInfo(const QString& message)
{
    if (guiStatusBar())
        guiStatusBar()->setMessage(message);
    guiDisplayInfo(message);
}

void YView::centerViewHorizontally( int column)
{
	/* TODO */
    // dbg() << "YView::centerViewHorizontally " << column << endl;
}

void YView::centerViewVertically( int line )
{
    if ( line == -1 )
        line = mainCursor.screenY();
    int newcurrent = 0;
    if ( line > mDrawBuffer.screenHeight() / 2 ) newcurrent = line - mDrawBuffer.screenHeight() / 2;
    alignViewVertically ( newcurrent );
}

void YView::bottomViewVertically( int line )
{
    int newcurrent = 0;
    if ( line >= mDrawBuffer.screenHeight() ) newcurrent = (line - mDrawBuffer.screenHeight()) + 1;
    alignViewVertically( newcurrent );
}

void YView::alignViewBufferVertically( int line )
{
	/* TODO */
}
void YView::alignViewVertically( int line )
{
	/* TODO */
    // dbg() << "YView::alignViewVertically " << line << endl;
}

/*
 * all the goto-like commands
 */


void YView::gotody( int nexty )
{
	// TODO
	YASSERT(false);
}

void YView::gotoy( int nexty )
{
	YASSERT(nexty >= 0);
	YASSERT(nexty < mBuffer->lineCount());
	if ( nexty < mDrawBuffer.topBufferLine() ||
			nexty > mDrawBuffer.bottomBufferLine() ) {
		dbg() << "gotoy("<<nexty<<") : out of drawbuffer" << endl;
		// scroll drawbuffer
		if ( nexty < mDrawBuffer.topBufferLine() ) {
			int delta = nexty - mDrawBuffer.topBufferLine();   // mDrawBuffer.topBufferLine+delta == nexty
			mDrawBuffer.verticalScroll(delta);
			updateBufferInterval(mDrawBuffer.topBufferLine(), mDrawBuffer.topBufferLine()-delta-1);
		} else {
			int previousBottomLine = mDrawBuffer.bottomBufferLine();
			int delta = nexty - mDrawBuffer.bottomBufferLine();
			mDrawBuffer.verticalScroll(delta);
			updateBufferInterval(previousBottomLine+1, nexty);
		}
		sendRefreshEvent(); //XXX optimize with guiScroll
	}
	mWorkDrawSection = mDrawBuffer.bufferDrawSection(nexty);
	workCursor.setBufferY(nexty);
	workCursor.setScreenY(mDrawBuffer.bufferDrawSectionScreenLine(nexty));
}

void YView::gotox( int nextx, bool forceGoBehindEOL )
{
	YASSERT(nextx >= 0);
	int shift = (mWorkDrawLine.bufferLength() == 0 || forceGoBehindEOL || mModePool->current()->isEditMode()) ? 1 : 0;
	nextx = qMin(mBuffer->getLineLength(workCursor.bufferY()) - 1 + shift, nextx);
	//TODO check this forceGoBehindEOL parameter...

	/* select targeted YDrawLine */
	int acc_x = 0;
	int dy = 0;
	foreach( mWorkDrawLine, mWorkDrawSection ) {
		if ( (acc_x + mWorkDrawLine.bufferLength() + shift) > nextx ) {
			// drawLine contains our destination
			break;
		} else if ( dy < mWorkDrawSection.count() - 1 ) {
			// destination is after drawLine, prepare next
			acc_x += mWorkDrawLine.bufferLength();
			dy += 1;
		}
	}

	workCursor.setScreenY(workCursor.screenY() + dy);

	int acc_dx = 0;

	QList<int>::const_iterator it = mWorkDrawLine.steps().constBegin();
	for( ; it!=mWorkDrawLine.steps().constEnd() && acc_x < nextx; ++acc_x, ++it ) {
		acc_dx += *it;
	}
	if ( shift && acc_x < nextx ) {
		acc_dx += 1;
		if ( acc_dx == mDrawBuffer.screenWidth() ) {
			acc_dx = 0;
			workCursor.setScreenY(workCursor.screenY()+1);
		}
		acc_x += 1;
	}
	workCursor.setBufferX(acc_x);
	workCursor.setScreenX(acc_dx);
}

void YView::gotodx( int nextx )
{
	//TODO: directly support nextx > screenWidth
	int shift = (mWorkDrawLine.bufferLength() == 0 || mModePool->current()->isEditMode()) ? 1 : 0;
	YASSERT(0 <= nextx && nextx < (mDrawBuffer.screenWidth()+shift));
	//TODO nextx = qMin(...);


	mWorkDrawLine = mWorkDrawSection.at(0);

	int acc_x = 0;
	int acc_dx = 0;
	foreach( int step, mWorkDrawLine.steps() ) {
		acc_dx += step;
		if ( acc_dx > nextx ) {
			break;
		}
		++acc_x;
	}
	if ( shift && acc_dx < nextx ) {
		acc_dx += 1;
		if ( acc_dx == mDrawBuffer.screenWidth() ) {
			acc_dx = 0;
			workCursor.setScreenY(workCursor.screenY()+1);
		}
		acc_x += 1;
	}
	workCursor.setScreenX(acc_dx);
	workCursor.setBufferX(acc_x);
}

void YView::initGoto( YViewCursor* viewCursor )
{
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

    if ( applyCursor ) {

        setPaintAutoCommit( false );

        mModePool->current()->cursorMoved( this );

        if ( !isColumnVisible( mainCursor.screenX(), mainCursor.screenY() ) ) {
            centerViewHorizontally( mainCursor.screenX( ) );
        }
        if ( !isLineVisible( mainCursor.screenY() ) ) {
			dbg() << "applyGoto: cursor is out of screen! TODO" << endl;
			YASSERT(false);
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
	/* TODO */
	return true;
}
bool YView::isLineVisible( int l ) const
{
	/* screen line */
	return l >= 0 and l < mDrawBuffer.screenWidth();
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
        at = ( ( *hl ) >= len ) ? &mHighlightAttributes[ 0 ] : &mHighlightAttributes[*hl]; //attributes pointed by line's attribute for current column
    }
    if ( opt_list && ( yl->data().at(col) == ' ' || yl->data().at(col) == tabChar ) )
        return blue;
    if ( at ) return at->textColor(); //textcolor :)
    return color_null;
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
        int col = stickyCol % mDrawBuffer.screenWidth();
        int deltaY = stickyCol / mDrawBuffer.screenWidth();
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
    if ( (delta < 0 && Y >= -delta) || (delta >= 0 && ( Y + delta ) < mBuffer->lineCount()) )
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
	/* TODO 
    stickyCol = ( viewCursor->lineHeight - 1 ) * mDrawBuffer.screenWidth() + viewCursor->screenX();
	*/
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
    return (QPoint)mainCursor.screen();
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
    YCursor bottomRight = YCursor(getColumnsVisible() - 1, getLinesVisible() - 1);
    return sel.clip(YInterval(YCursor(0,0), bottomRight));
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
    if ( m_paintAutoCommit == 0 || --m_paintAutoCommit == 0 ) {
        if ( keepCursor.valid() ) {
            mainCursor = keepCursor;
            keepCursor.invalidate();
            applyGoto( &mainCursor );
        }
        if ( ! mPaintSelection.isEmpty() ) {
            guiNotifyContentChanged(clipSelection(mPaintSelection));
        }
        abortPaintEvent();
    }
}
void YView::abortPaintEvent()
{
    keepCursor.invalidate();
    mPaintSelection.clear();
    setPaintAutoCommit();
}

void YView::sendCursor( YViewCursor cursor )
{
    keepCursor = cursor;
}
void YView::sendPaintEvent( const YInterval& i )
{
	if ( i.valid() ) {
		setPaintAutoCommit(false);
		mPaintSelection.addInterval(i);
		commitPaintEvent();
	}
}
void YView::sendPaintEvent( int curx, int cury, int curw, int curh )
{
    if ( curh == 0 ) {
        dbg() << "Warning: YView::sendPaintEvent with height = 0" << endl;
        return ;
    }
    sendPaintEvent(YInterval(YCursor(curx, cury), YCursor(curx + curw, cury + curh - 1)));
}
void YView::sendPaintEvent( const YCursor from, const YCursor to )
{
	sendPaintEvent(YInterval(from, to));
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
    mPaintSelection.addMap( map );
    commitPaintEvent();
}

void YView::sendRefreshEvent( )
{
    mPaintSelection.clear();
    sendPaintEvent(YInterval(YCursor(0,0), YBound(YCursor(0,mDrawBuffer.screenHeight()), true)));
}

void YView::removePaintEvent( const YCursor from, const YCursor to )
{
    mPaintSelection.delInterval( YInterval( from, to ) );
}

bool YView::stringHasOnlySpaces ( const QString& what )
{
    for (int i = 0 ; i < what.length(); i++)
        if ( !what.at(i).isSpace() ) {
            return false;
        }
    return true;
}

void YView::saveInputBuffer()
{
    // Only have special cases for length 1
    if ( mPreviousChars.count() == 1 ) {
        // We don't need to remember ENTER or ESC or CTRL-C
        if ( *mPreviousChars.begin() == Qt::Key_Enter
             || *mPreviousChars.begin() == Qt::Key_Return
             || *mPreviousChars.begin() == Qt::Key_Escape
             || *mPreviousChars.begin() == YKey(Qt::Key_C, Qt::ControlModifier) )
            return;

        // Provided we are not repeating the command don't overwrite
        if ( *mPreviousChars.begin() == Qt::Key_Period ) 
            return;
    }

    // Nothing odd, so go ahead and save copy
    mLastPreviousChars = mPreviousChars;
}

void YView::internalScroll( int dx, int dy )
{
    //mDrawBuffer.Scroll( dx, dy ); TODO
    guiScroll( dx, dy );
}

int YView::getCurrentTop() const
{
	return mDrawBuffer.topBufferLine();
}
int YView::getLinesVisible() const
{
	return mDrawBuffer.screenHeight();
}
int YView::getColumnsVisible() const
{
	return mDrawBuffer.screenWidth();
}

void YView::updateBufferInterval( const YInterval& bi )
{
	int last_bl = bi.toPos().line();
	if ( bi.to().opened() && bi.toPos().column() == 0 )
		--last_bl;
	return updateBufferInterval(bi.fromPos().line(), last_bl);
}

void YView::updateBufferInterval( int bl, int bl_last )
{
	YASSERT(bl <= bl_last);
	if ( mDrawBuffer.topBufferLine() > bl_last ||
			(mDrawBuffer.bottomBufferLine()+(mDrawBuffer.full()?0:1) < bl) ) {
		dbg() << "ignoring updateBufferInterval from line "<<bl<<" to " << bl_last << " ["<<mDrawBuffer.topBufferLine()<<" to "<<mDrawBuffer.bottomBufferLine()<<" full=" << mDrawBuffer.full()<<"]" << endl;
		return;
	}

	/* clipping */
	bl = qMax(bl, mDrawBuffer.topBufferLine());
	dbg() << "updateBufferInterval from line "<<bl<<" to " << bl_last << endl;

	setPaintAutoCommit(false);
	int nextScreenLine = 0;
	/* update requested lines */
	for( ; bl < mBuffer->lineCount() && bl <= bl_last && nextScreenLine < mDrawBuffer.screenHeight(); ++bl ) {
		nextScreenLine = setBufferLineContent(bl, mBuffer->yzline(bl));
	}
	if ( bl <= bl_last ) {
		deleteFromBufferLine(bl);
	}
	commitPaintEvent();
}


YDrawLine YView::drawLineFromYLine( const YLine* yl, int start_column ) {
	YDrawLine dl;

	QString data = yl->data();
    const uchar* hl = mHighlightAttributes ? yl->attributes() : NULL;

	QString text;
	QChar fillChar;
	YColor fg, bg, outline;
	YFont font;
	YzisAttribute* last_at = NULL;
	YzisAttribute* at = NULL;
	bool last_is_listchar = false;
	bool is_listchar;
	int drawLength;
	int column = start_column;

	for ( int i = 0; i < data.length(); ++i ) {

		text = data.at(i);

		fillChar = ' ';
		if ( text == tabChar ) {
			drawLength = tabstop - column % tabstop;
			/* column + drawLength = 0 mod tabstop */
		} else {
			drawLength = 1;
		}

		/* :set list support */
		is_listchar = opt_list && (text == " " || text == tabChar);
		if ( is_listchar ) {
			if ( text == " " ) {
				if ( stringHasOnlySpaces(data.mid(i)) && opt_listchars[ "trail" ].length() > 0 ) {
					text = opt_listchars["trail"][0];
				} else if ( opt_listchars["space"].length() > 0 ) {
					text = opt_listchars["space"][0];
				}
			} else if ( text == tabChar ) {
				if ( opt_listchars["tab"].length() > 0 ) {
					text = opt_listchars["tab"][0];
					if ( opt_listchars["tab"].length() > 1 ) {
						fillChar = opt_listchars["tab"][1];
					}
				}
			}
		}

		if ( drawLength > 1 ) {
			text = text.leftJustified(drawLength, fillChar);
		}

		/* syntax highlighting attributes */
		if ( hl ) {
			at = &mHighlightAttributes[*hl];
			++hl;
		}

		if ( i == 0 || last_at != at || is_listchar != last_is_listchar ) {
			if ( at ) {
				fg = at->textColor();
				bg = at->bgColor();
				font.setWeight(at->bold() ? YFont::Bold : YFont::Normal);
				font.setItalic(at->italic());
				font.setUnderline(at->underline());
				font.setOverline(at->overline());
				font.setStrikeOut(at->strikeOut());
				outline = at->outline();
			} else {
				fg = color_null;
				bg = color_null;
				font.setWeight(YFont::Normal);
				font.setItalic(false);
				font.setUnderline(false);
				font.setOverline(false);
				font.setStrikeOut(false);
				outline = color_null;
			}
			if ( is_listchar ) {
				fg = blue; // TODO: make custom
				outline = color_null; // TODO: make custom
			}
			dl.setColor(fg);
			dl.setBackgroundColor(bg);
			// TODO: setSelection
			// TODO: setOutline
			dl.setFont(font);
			last_at = at;
			last_is_listchar = is_listchar;
		}
		column += dl.push(text);
	}

	return dl;
}

int YView::setBufferLineContent( int bl, const YLine* yl )
{
	YDrawLine dl = drawLineFromYLine(yl);
	YDrawSection ds;
	if ( wrap ) {
		ds = dl.arrange(mDrawBuffer.screenWidth());
	} else {
		ds << dl;
	}

	int shift = 0;
	int dy = mDrawBuffer.setBufferDrawSection(bl, ds, &shift);
	int ndy = dy+ds.count();
	if ( shift != 0 ) {
		if ( shift < 0 ) {
			/* try to add new lines */
			int start_bl = mDrawBuffer.bottomBufferLine() + 1;
			if ( start_bl < mBuffer->lineCount() ) {
				updateBufferInterval(start_bl, mBuffer->lineCount()-1);	
			}
		}
		sendPaintEvent(YInterval(YCursor(0,ndy), YBound(YCursor(0,mDrawBuffer.screenHeight()), true))); //XXX optimize with guiScroll
	}
	sendPaintEvent(YInterval(YCursor(0,dy), YBound(YCursor(0,ndy), true)));
	return ndy;
}
void YView::deleteFromBufferLine( int bl )
{
	YInterval affected = mDrawBuffer.deleteFromBufferDrawSection(bl);
	sendPaintEvent(affected);
}

void YView::guiPaintEvent( const YSelection& drawMap )
{
    if ( drawMap.isEmpty() )
        return ;

    dbg() << "guiPaintEvent" << drawMap << endl;

    guiPreparePaintEvent();

	bool show_numbers = getLocalBooleanOption("number");
	if ( show_numbers ) {
		guiDrawSetMaxLineNumber(mBuffer->lineCount());
	}

	int cur_line = -1;
	foreach( YInterval di, drawMap.map() ) {
		for ( YDrawBufferIterator it = mDrawBuffer.iterator(di); it.isValid(); it.next() ) {
			if ( show_numbers && cur_line != it.screenLine() ) {
				guiDrawSetLineNumber(it.screenLine(), it.bufferLine() + 1, it.lineHeight());
				cur_line = it.screenLine();
			}
			const YDrawCellInfo ci = it.drawCellInfo();
			switch ( ci.type ) {
				case YDrawCellInfo::Data :
					guiDrawCell(ci.pos, ci.cell);
					break;
				case YDrawCellInfo::EOL :
					guiDrawClearToEOL(ci.pos, ci.cell);
					break;
			}
		}
	}

	if ( !mDrawBuffer.full() ) { 
		/* may be fake lines ? */

		YDrawCell fl;
		fl.c = "~";
		fl.fg = YColor("cyan");

		YInterval fake(YCursor(0,mDrawBuffer.currentHeight()),YCursor(mDrawBuffer.screenWidth()-1,mDrawBuffer.screenHeight()-1));
		foreach( YInterval di, drawMap.map() ) {
			YInterval i = di.intersection(fake);
			if ( !i.valid() ) {
				continue;
			}
			for ( int cur_dy = i.fromPos().line(); cur_dy <= i.toPos().line(); ++cur_dy ) {
				if ( show_numbers ) {
					guiDrawSetLineNumber(cur_dy, -1, 0); /* clear line number */
				}
				guiDrawCell(YCursor(0,cur_dy), fl);
				guiDrawClearToEOL(YCursor(1,cur_dy), mDrawBuffer.EOLCell());
			}
		}
	}

    guiEndPaintEvent();
}

