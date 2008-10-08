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

static const QChar tabChar('\t');
static const YColor color_null;
static const YColor blue(Qt::blue);

#define dbg() yzDebug("YView")
#define err() yzError("YView")

YViewIface::~YViewIface()
{}

/**
 * class YView
 */

static int nextId = 1;

YView::YView(YBuffer *_b, YSession *sess, int cols, int lines)
		:
	mDrawBuffer(this, cols, lines),
	mPreviousChars(""),mLastPreviousChars(""),
	mSession(sess),
	mBuffer(_b),
	mMainCursor(),
	mStickyColumn(0),
	mSelectionPool(),
	mPaintSelection(),
	id(nextId++)
{
	YASSERT(mSession);
    YASSERT(mBuffer);
    dbg().SPrintf("YView(%s, cols=%d, lines=%d)", qp(mBuffer->toString()), cols, lines );
    dbg() << "New View created with UID: " << getId() << endl;

    mLineSearch = new YLineSearch(this); //TODO de-pointer-ize
    mModePool = new YModePool(this); //TODO de-pointer-ize
    mFoldPool = new YZFoldPool(this); //TODO de-pointer-ize

	updateInternalAttributes();

    resetPaintEvent();

    mBuffer->addView(this);
}

YView::~YView()
{
    dbg() << "~YView(): Deleting view " << id << endl;
    mModePool->stop();
    mBuffer->saveYzisInfo(this);
    mBuffer->rmView(this); //make my buffer forget about me
    if (mBuffer->views().isEmpty()) {
        // last view deleted, delete the buffer
		/* TODO: THIS IS BAD!!!! get rid of this */
        YSession::self()->deleteBuffer( mBuffer );
    }

    delete mLineSearch;
    delete mModePool;
    delete mFoldPool;
}

QString YView::toString() const
{
    QString s;
    s.sprintf("View(this=%p id=%d buffer='%s')", this, getId(), qp(buffer()->fileNameShort()) );
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

	setPaintAutoCommit(false);

	updateBufferInterval(YInterval(YCursor(0,0), YBound(YCursor(0, mBuffer->lineCount()), true)));
	sendRefreshEvent();

	gotoViewCursor(viewCursorFromLinePosition(old_pos));

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
    mBuffer->action()->replaceLine( this, YCursor( 0, mMainCursor.line() ), currentLine );
	gotoViewCursor(viewCursorFromLinePosition(mMainCursor.line(), currentLine.length()));
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
    int ypos = mMainCursor.line();
    QString currentLine = mBuffer->textline( ypos );
    QRegExp rxLeadingWhiteSpace( "^([ \t]*).*$" );
    if ( !rxLeadingWhiteSpace.exactMatch( currentLine ) ) {
        return ; //Shouldn't happen
    }
    QString indentString = rxLeadingWhiteSpace.cap( 1 );
    if ( mMainCursor.position() == currentLine.length() && currentLine.trimmed().endsWith( indentMarker ) ) {
        //dbg() << "Indent marker found" << endl;
        // This should probably be tabstop...
        indentString.append( "\t" );
    }
    //dbg() << "Indent string = \"" << indentString << "\"" << endl;
    mBuffer->action()->insertNewLine( this, mMainCursor.buffer() );
    ypos++;
    mBuffer->action()->replaceLine( this, ypos, indentString + mBuffer->textline( ypos ).trimmed() );
    gotoLinePosition(ypos , indentString.length());
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

    int y = mMainCursor.line();

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

    if ( guiStatusBar() ) {
        guiStatusBar()->setLineInfo(y+1, mMainCursor.position()+1, mMainCursor.column()+1, percentage);
	}
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
    QString filename = buffer()->fileName();
    if (guiStatusBar())
        guiStatusBar()->setFileName(filename);
    guiUpdateFileName();
}

void YView::updateFileInfo()
{
    if (guiStatusBar())
        guiStatusBar()->setFileInfo(buffer()->fileIsNew(), buffer()->fileIsModified());
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
	//TODO
#if 0
    if ( line == -1 )
        line = mMainCursor.screenY();
    int newcurrent = 0;
    if ( line > mDrawBuffer.screenHeight() / 2 ) newcurrent = line - mDrawBuffer.screenHeight() / 2;
    alignViewVertically ( newcurrent );
#endif
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



YViewCursor YView::viewCursorFromLinePosition( int line, int position ) 
{
	//TODO: behindEOL
	YASSERT(line >= 0);
	YASSERT(line < mBuffer->lineCount());
	YASSERT(position >= 0);
	int sid, lid, cid, bshift, column;
	mDrawBuffer.targetBufferLine(line, &sid);
	int max_position = qMax(0, mBuffer->getLineLength(line) - 1);
	bool goBehindEnd = false;
	if ( position > max_position ) {
		position = max_position;
		//TODO goBehindEnd
	}
	mDrawBuffer.targetBufferColumn(position, sid, &lid, &cid, &bshift, &column);
	//TODO goBehindEnd
	dbg() << "viewCursorFromLinePosition(" << line<<","<<position<<") => line,position,column = " << line<<","<<position<<","<<column<< endl;
	return YViewCursor(line, position, column);
}

YViewCursor YView::viewCursorFromRowColumn( int row, int column ) const
{
	//TODO: behindEOL
	YASSERT(row >= 0);
	YASSERT(row < mDrawBuffer.screenHeight());
	YASSERT(column >= 0);
	YASSERT(column < mDrawBuffer.screenWidth());
	int sid, lid, line;
	mDrawBuffer.targetScreenLine(row, &sid, &lid, &line);
	int cid, sshift, position;
	mDrawBuffer.targetScreenColumn(column, sid, lid, &cid, &sshift, &position);
	return YViewCursor(line, position, column);
}

YViewCursor YView::viewCursorFromLineColumn( int line, int column )
{
	//TODO: behindEOL
	YASSERT(line >= 0);
	YASSERT(line < mBuffer->lineCount());
	YASSERT(column >= 0);
	int sid;
	mDrawBuffer.targetBufferLine(line, &sid);
	int lid = column / mDrawBuffer.screenWidth();
	int scol = column % mDrawBuffer.screenWidth();
	int cid, sshift, position;
	mDrawBuffer.targetScreenColumn(scol, sid, lid, &cid, &sshift, &position);
	return YViewCursor(line, position, column);
}

/*
 * Sticky column support
 */
#define STICK_ENDLINE -1
YViewCursor YView::viewCursorFromStickedLine( int line )
{
	if ( mStickyColumn == STICK_ENDLINE ) {
		return viewCursorFromLinePosition(line, qMax(0,mBuffer->getLineLength(line)-1));
	} else {
		return viewCursorFromLineColumn(line, mStickyColumn);
	}
}
void YView::stickToColumn()
{
	mStickyColumn = mMainCursor.column();
}
void YView::stickToEOL()
{
	mStickyColumn = STICK_ENDLINE;
}


void YView::gotoViewCursor( const YViewCursor& cursor )
{
	mMainCursor = cursor;
	int scroll_horizontal, scroll_vertical;
	if ( mDrawBuffer.scrollForViewCursor(mMainCursor, &scroll_horizontal, &scroll_vertical) ) {
		guiScroll(scroll_horizontal, scroll_vertical);
	}

	mModePool->current()->cursorMoved( this );
	updateCursor();
}

void YView::gotoLinePosition( int line, int position )
{
	gotoViewCursor(viewCursorFromLinePosition(line, position));
}
void YView::gotoLinePositionAndStick( int line, int position ) 
{
	gotoLinePosition(line, position);
	stickToColumn();
}
void YView::gotoLinePosition( const YCursor& buffer )
{
	gotoLinePosition(buffer.line(), buffer.column());
}
void YView::gotoLinePositionAndStick( const YCursor& buffer )
{
	gotoLinePositionAndStick(buffer.line(), buffer.column());
}

void YView::gotoRowColumn( int row, int column )
{
	gotoViewCursor(viewCursorFromRowColumn(row, column));
}
void YView::gotoRowColumn( const YCursor& screen )
{
	gotoRowColumn(screen.line(), screen.column());
}

YViewCursor YView::viewCursorMoveVertical( int ticks )
{
	int line = qMin(mBuffer->lineCount()-1, qMax(0, mMainCursor.line() + ticks));
	return viewCursorFromStickedLine(line);
}

YViewCursor YView::viewCursorMoveHorizontal( int ticks, bool wrap, bool* stopped )
{
	//TODO: check behindEOL
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

	return viewCursorFromLinePosition(line, position);
}

void YView::applyStartPosition( const YCursor pos )
{
    if ( pos.y() >= 0 ) {
		YViewCursor dest;
        if ( pos.x() >= 0 ) {
			dest = viewCursorFromLinePosition(pos);
        } else {
			dest = viewCursorFromStickedLine(pos.y());
        }
		gotoViewCursor(dest);
		stickToColumn();
    }
}

QString YView::append ()
{
    mModePool->change( YMode::ModeInsert );
    gotoLinePositionAndStick(mMainCursor.line() , mMainCursor.position() + 1);
    return QString();
}

void YView::commitUndoItem()
{
    mBuffer->undoBuffer()->commitUndoItem(mMainCursor.position(), mMainCursor.line());
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
	return l >= 0 and l < mDrawBuffer.screenHeight();
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
	int target_line = qMin(qMax(0, mMainCursor.line() + delta), mBuffer->lineCount());
	int target_column = mMainCursor.column();
	// TODO: use ConstIterator
	return QString();
}


void YView::commitNextUndo()
{
    mBuffer->undoBuffer()->commitUndoItem( mMainCursor.position(), mMainCursor.line() );
}


const YCursor YView::getRowColumnCursor() const
{
	int col = currentColumn() % getColumnsVisible();
	int row = currentLine() - mDrawBuffer.screenTopBufferLine() + currentColumn() / getColumnsVisible();
	return YCursor(col, row);
}
const YCursor YView::getLinePositionCursor() const
{
    return mMainCursor.buffer();
}
const YCursor YView::getLineColumnCursor() const
{
	return YCursor(currentColumn(), currentLine());
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

bool YView::stringHasOnlySpaces ( const QString& what ) const
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
	return mDrawBuffer.screenTopBufferLine();
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
	if ( mDrawBuffer.firstBufferLine() > bl_last || mDrawBuffer.lastBufferLine()+(mDrawBuffer.full()?0:1) < bl ) {
		dbg() << "ignoring updateBufferInterval from line "<<bl<<" to " << bl_last << " ["<<mDrawBuffer.firstBufferLine()<<" to "<<mDrawBuffer.lastBufferLine()<<" full=" << mDrawBuffer.full()<<"]" << endl;
		return;
	}

	/* clipping */
	bl = qMax(bl, mDrawBuffer.firstBufferLine());
	dbg() << "updateBufferInterval from line "<<bl<<" to " << bl_last << endl;

	setPaintAutoCommit(false);

	/* delete extra lines */
	if ( bl_last >= mBuffer->lineCount() ) {
		deleteFromBufferLine(mBuffer->lineCount());
	}
	bl_last = qMin(bl_last, mBuffer->lineCount()-1);

	/* update requested lines */
	for( ; bl <= bl_last; ++bl ) {
		setBufferLineContent(bl);
	}

	commitPaintEvent();
}


YDrawLine YView::drawLineFromYLine( const YLine* yl, int start_column ) const
{
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

YDrawSection YView::drawSectionOfBufferLine( int bl ) const {
	const YLine* yl = mBuffer->yzline(bl);
	YDrawLine dl = drawLineFromYLine(yl);
	YDrawSection ds;
	if ( wrap ) {
		ds = dl.arrange(mDrawBuffer.screenWidth());
	} else {
		ds << dl;
	}
	return ds;
}

void YView::setBufferLineContent( int bl )
{
	YDrawSection ds = drawSectionOfBufferLine(bl);
	YInterval affected = mDrawBuffer.setBufferDrawSection(bl, ds);
	sendPaintEvent(affected);
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

