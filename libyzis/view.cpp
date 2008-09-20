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

#define STICK_ENDLINE -1

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
	mMainCursor(), workCursor(),
	mSelectionPool(),
	mPaintSelection(),
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

    resetPaintEvent();
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

    YCursor old_pos = mMainCursor.buffer();
    mMainCursor.reset();

	setPaintAutoCommit(false);

	updateBufferInterval(YInterval(YCursor(0,0), YBound(YCursor(0, mBuffer->lineCount()), true)));
	sendRefreshEvent();

    gotoxy( &mMainCursor, old_pos );

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
    mBuffer->action()->replaceLine( this, YCursor( 0, mMainCursor.bufferY() ), currentLine );
    gotoxy( currentLine.length(), mMainCursor.bufferY() );
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
    int ypos = mMainCursor.bufferY();
    QString currentLine = mBuffer->textline( ypos );
    QRegExp rxLeadingWhiteSpace( "^([ \t]*).*$" );
    if ( !rxLeadingWhiteSpace.exactMatch( currentLine ) ) {
        return ; //Shouldn't happen
    }
    QString indentString = rxLeadingWhiteSpace.cap( 1 );
    if ( mMainCursor.bufferX() == currentLine.length() && currentLine.trimmed().endsWith( indentMarker ) ) {
        //dbg() << "Indent marker found" << endl;
        // This should probably be tabstop...
        indentString.append( "\t" );
    }
    //dbg() << "Indent string = \"" << indentString << "\"" << endl;
    mBuffer->action()->insertNewLine( this, mMainCursor.buffer() );
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

    int y = mMainCursor.bufferY();

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
        line = mMainCursor.screenY();
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



YViewCursor YView::viewCursorFromLinePosition( int line, int position ) const
{
	YASSERT(line >= 0);
	YASSERT(line < mBuffer->lineCount());
	YASSERT(position >= 0);
	int sid, lid, cid, bshift, column;
	mDrawBuffer->targetBufferLine(line, &sid);
	mDrawBuffer->targetBufferColumn(position, sid, &lid, &cid, &bshift, &column);
	return YViewCursor(line, position, column);
}

YViewCursor YView::viewCursorFromLineColumn( int line, int column ) const
{
	YASSERT(line >= 0);
	YASSERT(line < mBuffer->lineCount());
	YASSERT(column >= 0);
	int sid;
	mDrawBuffer->targetBufferLine(line, &sid);
	int lid = column / mDrawBuffer->screenWidth();
	int scol = column % mDrawBuffer->screenWidth();
	int cid, sshift, position;
	mDrawbuffer->targetScreenColumn(scol, sid, lid, &cid, &sshift, &position);
	return YViewCursor(line, position, column);
}

YViewCursor YView::viewCursorFromStickedLine( int line ) const
{
	if ( stickyColumn == STICK_ENDLINE ) {
		return viewCursorFromLinePosition(line, mBuffer->getLineLength(line)-1);
	} else {
		return viewCursorFromLineColumn(line, column);
	}
}

/*
 * all the goto-like commands
 */



void YView::applyGoto()
{
	//TODO: scroll
#if 0
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


        if ( !isColumnVisible( mMainCursor.screenX(), mMainCursor.screenY() ) ) {
            centerViewHorizontally( mMainCursor.screenX( ) );
        }
        if ( !isLineVisible( mMainCursor.screenY() ) ) {
			dbg() << "applyGoto: cursor is out of screen! TODO" << endl;
			YASSERT(false);
        }
        commitPaintEvent();
    }
#endif
	mModePool->current()->cursorMoved( this );
	updateCursor();
}

void YView::gotoViewCursor( const YViewCursor& cursor )
{
	mMainCursor = cursor;
	applyGoto();
}


// These all return whether the motion was stopped
bool YView::moveDown( int nb_lines, bool applyCursor )
{
    return moveDown( &mMainCursor, nb_lines, applyCursor );
}
bool YView::moveDown( YViewCursor* viewCursor, int nb_lines, bool applyCursor )
{
    int destRequested = mFoldPool->lineAfterFold( viewCursor->bufferY() + nb_lines );
    gotoStickyCol( viewCursor, qMin( destRequested, mBuffer->lineCount() - 1 ), applyCursor );
    return destRequested > (mBuffer->lineCount() - 1);
}
bool YView::moveUp( int nb_lines, bool applyCursor )
{
    return moveUp( &mMainCursor, nb_lines, applyCursor );
}
bool YView::moveUp( YViewCursor* viewCursor, int nb_lines, bool applyCursor )
{
    int destRequested = viewCursor->bufferY() - nb_lines;
    gotoStickyCol( viewCursor, qMax( destRequested, 0 ), applyCursor );
    return destRequested < 0;
}

YViewCursor YView::moveHorizontal( int ticks, bool wrap, bool* stopped )
{
	int line = mMainCursor.line();
	int position = mMainCursor.position() + ticks;
	bool my_stopped = false;
	if ( position < 0 ) {
		if ( wrap ) {
			while ( position < 0 && line >= 1 ) {
				// go one line up
				line -= 1;
				position = mBuffer->getLineLength(line) + position + 1;
			}
			if ( position < 0 ) {
				my_stopped = true;
				position = 0;
			}
		} else {
			position = 0;
		}
	} else if ( position >= mBuffer->getLineLength(line) ) {
		int max_line = mBuffer->lineCount() - 1;
		if ( wrap && line < max_line) {
			int line_length = mBuffer->getLineLength(line);
			do {
				position -= line_length;
				line += 1;
				line_length = mBuffer->getLineLength(line);
			} while ( position >= line_length && line < max_line );
			if ( position >= line_length ) {
				my_stopped = true;
				position = line_length - 1;
			}
		} else {
			position = mBuffer->getLineLength(line) - 1;
		}
	}
	if ( stopped != NULL ) *stopped = my_stopped;
	YViewCursor dest = viewCursorFromLinePosition(line, position);
	if ( applyCursor ) {
		gotoViewCursor(dest);
		updateStickyColumn();
	}
	return dest;
}

QString YView::moveToFirstNonBlankOfLine( )
{
    return moveToFirstNonBlankOfLine( &mMainCursor );
}

QString YView::moveToFirstNonBlankOfLine( YViewCursor* viewCursor, bool applyCursor )
{
    //execute the code
    gotoxy( viewCursor, mBuffer->firstNonBlankChar(viewCursor->bufferY()) , viewCursor->bufferY(), applyCursor );
    // if ( viewCursor == mMainCursor ) UPDATE_STICKY_COL;
    if ( applyCursor )
        updateStickyCol( viewCursor );

    //return something
    return QString();
}

void YView::gotoLastLine()
{
    gotoLastLine( &mMainCursor );
}
void YView::gotoLastLine( YViewCursor* viewCursor, bool applyCursor )
{
    gotoLine( viewCursor, mBuffer->lineCount() - 1, applyCursor );
}
void YView::gotoLine( int line )
{
    gotoLine( &mMainCursor, line );
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

void YView::applyStartPosition( const YCursor pos )
{
    if ( pos.y() >= 0 ) {
        if ( pos.x() >= 0 ) {
            gotoxyAndStick( pos );
        } else {
            gotoLine( pos.y() );
        }
    }
}

QString YView::append ()
{
    mModePool->change( YMode::ModeInsert );
    gotoxyAndStick(mMainCursor.bufferX() + 1, mMainCursor.bufferY() );
    return QString();
}

void YView::commitUndoItem()
{
    mBuffer->undoBuffer()->commitUndoItem(mMainCursor.bufferX(), mMainCursor.bufferY());
}


YRawData YView::setSelection( yzis::SelectionType type, const YInterval& bufferInterval )
{
	YRawData selectedData;
	setPaintAutoCommit(false);
	if ( type == yzis::SelectionVisual ) {
		dbg() << "setSelection[Visual] = " << bufferInterval << endl;
		if ( mSelectionPool.contains(type) ) {
			sendPaintEvent(mDrawBuffer.delSelection(type, mSelectionPool[type], yzis::BufferInterval));
		}
		mSelectionPool[type] = bufferInterval;
		sendPaintEvent(mDrawBuffer.addSelection(type, mSelectionPool[type], yzis::BufferInterval));
		selectedData = mBuffer->dataRegion(bufferInterval);
	} else {
		//TODO
		YASSERT(false);
	}
	commitPaintEvent();
	return selectedData;
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

QString YView::getCharBelow( int delta )
{
	int target_line = qMin(qMax(0, mMainCursor.bufferLine() + delta), mBuffer->lineCount());
	int target_column = mMainCursor.column();
	// TODO: use ConstIterator
	return QString();
}

void YView::stickToColumn()
{
	mStickyColumn = mMainCursor.column();
}
void YView::stickToEOL()
{
	mStickColumn = STICK_ENDLINE;
}

void YView::commitNextUndo()
{
    mBuffer->undoBuffer()->commitUndoItem( mMainCursor.bufferX(), mMainCursor.bufferY() );
}
const YCursor YView::getCursor() const
{
    return mMainCursor.screen();
}
const YCursor YView::getBufferCursor() const
{
    return mMainCursor.buffer();
}
YCursor YView::getRelativeScreenCursor() const
{
    return (QPoint)mMainCursor.screen();
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
        if ( !mPaintSelection.isEmpty() ) {
            guiNotifyContentChanged(clipSelection(mPaintSelection));
        }
        resetPaintEvent();
    }
}
void YView::resetPaintEvent()
{
    mPaintSelection.clear();
    setPaintAutoCommit();
}

void YView::sendPaintEvent( const YSelection& i )
{
	setPaintAutoCommit(false);
	mPaintSelection.addMap(s.map());
	commitPaintEvent();
}
void YView::sendPaintEvent( const YInterval& i )
{
	if ( i.valid() ) {
		setPaintAutoCommit(false);
		mPaintSelection.addInterval(i);
		commitPaintEvent();
	}
}
void YView::sendRefreshEvent( )
{
    mPaintSelection.clear();
    sendPaintEvent(YInterval(YCursor(0,0), YBound(YCursor(0,mDrawBuffer.screenHeight()), true)));
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
		column += dl.step(text);
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
		for ( YDrawBufferConstIterator it = mDrawBuffer.const_iterator(di, yzis::ScreenInterval); it.isValid(); it.next() ) {
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
		fl.step("~");
		fl.setForegroundColor(YColor("cyan"));

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

