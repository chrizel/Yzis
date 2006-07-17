/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>.
 *  Copyright (C) 2003-2006 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
 *  Copyright (C) 2005 Erlend Hamberg <ehamberg@online.no>
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

#include "view.h"

#include "portability.h"
#include <cstdlib>
#include <ctype.h>
#include <math.h>
#include "viewcursor.h"
#include "debug.h"
#include "undo.h"
#include "cursor.h"
#include "internal_options.h"
#include "registers.h"
#include "buffer.h"
#include "swapfile.h"
#include "mark.h"
#include "action.h"
#include "session.h"
#include "linesearch.h"
#include "mapping.h"
#include "folding.h"

using namespace yzis;

#define STICKY_COL_ENDLINE -1

#define GET_STRING_WIDTH( s ) ( isFontFixed ? s.length() : stringWidth( s ) )
#define GET_CHAR_WIDTH( c ) ( isFontFixed ? 1 : charWidth( c ) )

static const QChar tabChar( '\t' );

static YZColor color_null;
static YZColor blue( Qt::blue );

/**
 * class YZView
 */
 
static int nextId = 1;

YZView::YZView(YZBuffer *_b, YZSession *sess, int lines) 
	:  m_drawBuffer(), id( nextId++ )
{
	yzDebug() << "New View created with UID : " << id << endl;
	YZASSERT( _b ); YZASSERT( sess );
	mSession = sess;
	mBuffer	= _b;
	mLineSearch = new YZLineSearch( this );
	mLinesVis = lines;
	mColumnsVis = 0;

	mModePool = new YZModePool( this );

	mainCursor = new YZViewCursor( this );
	workCursor = new YZViewCursor( this );
	keepCursor = new YZViewCursor( this );

	scrollCursor = new YZViewCursor( this );
	/* start of visual mode */
	mVisualCursor = new YZViewCursor( this );

	mFoldPool = new YZFoldPool( this );

	origPos = new YZCursor();
	beginChanges = new YZCursor();

	m_drawBuffer.setCallback( this );

	stickyCol = 0;

	QString line = mBuffer->textline(scrollCursor->bufferY());

	reverseSearch=false;
	viewInformation.l = viewInformation.c1 = viewInformation.c2 = 0;
	viewInformation.percentage = "";
	mPreviousChars = "";
	mLastPreviousChars = "";

	mPaintSelection = new YZSelection("PAINT");
	selectionPool = new YZSelectionPool();
	setFixedFont( true );

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

	mModePool->change( YZMode::MODE_COMMAND );
}

YZView::~YZView() {
	mModePool->stop();
//	yzDebug() << "YZView : Deleting view " << id << endl;
	mBuffer->saveYzisInfo(this);
	mBuffer->rmView(this); //make my buffer forget about me

	delete mainCursor;
	delete scrollCursor;
	delete keepCursor;
	delete workCursor;
	delete origPos;
	delete mVisualCursor;
	delete selectionPool;
	delete mPaintSelection;
	delete beginChanges;
	delete mLineSearch;
	delete mModePool;
	delete mFoldPool;
}

void YZView::setupKeys() {
	mModePool->registerModifierKeys();
}

void YZView::setVisibleArea(int c, int l, bool refresh) {
	yzDebug() << "YZView::setVisibleArea(" << c << "," << l << ");" << endl;
	mLinesVis = l;
	mColumnsVis = c;
	if( refresh )
		recalcScreen();
}

void YZView::refreshScreen() {
	opt_schema = getLocalIntegerOption( "schema" );
	opt_list = getLocalBooleanOption( "list" );
	opt_listchars = getLocalMapOption( "listchars" );
	sendRefreshEvent();
}
void YZView::recalcScreen( ) {
	tabstop = getLocalIntegerOption( "tabstop" );
	wrap = getLocalBooleanOption( "wrap" );
	rightleft = getLocalBooleanOption( "rightleft" );

	YZCursor old_pos = scrollCursor->buffer();
	scrollCursor->reset();
	if ( wrap ) old_pos.setX( 0 );
	gotoxy( scrollCursor, old_pos.x(), old_pos.y(), false );

	old_pos = mainCursor->buffer();
	mainCursor->reset();
	gotoxy( mainCursor, old_pos.x(), old_pos.y() );

	sendRefreshEvent();
}

void YZView::sendMultipleKey(const QString& _keys) {
	if (mModePool->current()->mapMode() & cmdline) {
		mModePool->change( YZMode::MODE_COMMAND );
	}
	QString keys = _keys;
	yzDebug() << "YZView::sendMultipleKey " << keys << endl;
	for ( int i = 0 ; i < keys.length(); ) {
		QString key = keys.mid( i );
		yzDebug() << "Handling key : " << key << endl;
		//exception : in SEARCH, SEARCH_BACKWARD and EX mode we dont send keys immediately
		if (mModePool->current()->mapMode() & cmdline) {
			if ( key.startsWith( "<ESC>" ) ) {
				sendKey ( "<ESC>" );
				continue;
			} else if ( key.startsWith( "<ENTER>" ) ) {
				sendKey ( "<ENTER>" );
				i+=7;
				continue;
			} else if ( key.startsWith( "<UP>" ) ) {
				sendKey ( "<UP>" );
				i+=4;
				continue;
			} else if ( key.startsWith( "<DOWN>" ) ) {
				sendKey ( "<DOWN>" );
				i+=6;
				continue;
			} else {
				setCommandLineText( getCommandLineText() + key.mid(0,1) );
				yzDebug() << "Set commandline : " << getCommandLineText() + key.mid( 0,1 ) << endl;
				i++;
				continue;
			}
		}
		if ( key.startsWith( "<CTRL>" ) ) {
			yzDebug() << "Sending " << key.mid(6,1) << endl;
			sendKey (key.mid( 6,1 ), "<CTRL>" );
			i+=7;
			continue;
		} else if ( key.startsWith( "<ALT>" ) ) {
			sendKey (key.mid( 5,1 ), "<ALT>" );
			i+=6;
			continue;
		} else if ( key.startsWith( "<SHIFT>" ) ) {
			sendKey (key.mid( 7,1 ), "<SHIFT>" );
			i+=8;
			continue;
		} else if ( key.startsWith( "<ESC>" ) ) {
			sendKey ( "<ESC>" );
			i+=5;
			continue;
		} else if ( key.startsWith( "<ENTER>" ) ) {
			sendKey ( "<ENTER>" );
			i+=7;
			continue;
		} else if ( key.startsWith( "<TAB>" ) ) {
			sendKey ( "<TAB>" );
			i+=5;
			continue;
		} else if ( key.startsWith( "<UP>" ) ) {
			sendKey ( "<UP>" );
			i+=4;
			continue;
		} else if ( key.startsWith( "<DOWN>" ) ) {
			sendKey ( "<DOWN>" );
			i+=6;
			continue;
		} else if ( key.startsWith( "<RIGHT>" ) ) {
			sendKey ( "<RIGHT>" );
			i+=7;
			continue;
		} else if ( key.startsWith( "<LEFT>" ) ) {
			sendKey ( "<LEFT>" );
			i+=6;
			continue;
		} else if ( key.startsWith( "<DEL>" ) ) {
			sendKey ( "<DEL>" );
			i+=5;
			continue;
		} else {
			sendKey( key.mid( 0,1 ) );
			i++;
		}
	}
}

void YZView::displayIntro() {
	mModePool->change( YZMode::MODE_INTRO );
}

void YZView::sendKey( const QString& _key, const QString& _modifiers) {
//	yzDebug() << "YZView :: sendKey : " << _key << " mod=" << _modifiers << endl;

	QString key=_key;
	QString modifiers=_modifiers;
	if ( _key == "<SHIFT>" || _key == "<CTRL>" || _key == "<ALT>" ) return; //we are not supposed to received modifiers in key

	if ( mRegs.count() > 0 ) {
		for ( int ab = 0 ; ab < mRegs.size(); ++ab ) {
			QString newReg = modifiers + _key;
			QStringList curReg = YZSession::me->getRegister( mRegs.at(ab) );
			if ( curReg.size() > 0 )
				newReg.prepend( curReg[ 0 ] );
			YZSession::me->setRegister( mRegs.at(ab), QStringList( newReg ) );
		}
	}

	/** rightleft mapping **/
	if ( rightleft && ( mModePool->current()->mapMode() & (visual|normal) ) ) {
#define SWITCH_KEY( a, b ) \
	if ( key == a ) key = b; \
	else if ( key == b ) key = a
		SWITCH_KEY( "<RIGHT>", "<LEFT>" );
		SWITCH_KEY( "h", "l" );
	}

	if ( modifiers.contains ("<SHIFT>")) {//usefull ?
		key = key.toUpper();
		modifiers.remove( "<SHIFT>" );
	}

	mPreviousChars += modifiers + key;

	setPaintAutoCommit( false );
	mModePool->sendKey( key, modifiers );
	commitPaintEvent();
}

YZSelectionMap YZView::visualSelection() {
	return selectionPool->visual()->bufferMap();
}

void YZView::reindent( int X, int Y ) {
	yzDebug() << "Reindent " << endl;
	QRegExp rx("^(\\t*\\s*\\t*\\s*).*$"); //regexp to get all tabs and spaces
	QString currentLine = mBuffer->textline( Y ).trimmed();
	bool found = false;
	YZCursor cur( X, Y );
	YZCursor match = mBuffer->action()->match(this, cur, &found);
	if ( !found ) return;
	yzDebug() << "Match found on line " << match.y() << endl;
	QString matchLine = mBuffer->textline( match.y() );
	if ( rx.exactMatch( matchLine ) )
		currentLine.prepend( rx.cap( 1 ) ); //that should have all tabs and spaces from the previous line
	mBuffer->action()->replaceLine( this, YZCursor( 0, mainCursor->bufferY() ), currentLine );
	gotoxy( currentLine.length(), mainCursor->bufferY() );
}

/*
* Right now indent just copies the leading whitespace of the current line into the
* new line this isn't smart as it will duplicate extraneous spaces in indentation
* rather than giving consistent depth changes/composition based on user settings.
*/
void YZView::indent() {
	//yzDebug() << "Entered YZView::indent" << endl;
	QString indentMarker = "{"; // Just use open brace for now user defined (BEGIN or whatever) later
	int ypos = mainCursor->bufferY();
	QString currentLine = mBuffer->textline( ypos );
	QRegExp rxLeadingWhiteSpace( "^([ \t]*).*$" );
	if ( !rxLeadingWhiteSpace.exactMatch( currentLine ) ) {
		return; //Shouldn't happen
	}
	QString indentString = rxLeadingWhiteSpace.cap( 1 );
	if ( mainCursor->bufferX() == currentLine.length() && currentLine.trimmed().endsWith( indentMarker ) ) {
		//yzDebug() << "Indent marker found" << endl;
		// This should probably be tabstop...
		indentString.append( "\t" );
	}
	//yzDebug() << "Indent string = \"" << indentString << "\"" << endl;
	mBuffer->action()->insertNewLine( this, mainCursor->buffer() );
	ypos++;
	mBuffer->action()->replaceLine( this, ypos, indentString + mBuffer->textline( ypos ).trimmed() );
	gotoxy( indentString.length(), ypos );
	//yzDebug() << "Leaving YZView::indent" << endl;
}

QString YZView::centerLine( const QString& s ) {
	QString spacer = "";
	int nspaces = mColumnsVis > s.length() ? mColumnsVis - s.length() : 0;
	nspaces /= 2;
	spacer.fill( ' ', nspaces );
	spacer.append( s );
	return spacer;
}

void YZView::updateCursor() {
	int lasty = -1;
	viewInformation.percentage = _( "All" );
	int y = mainCursor->bufferY();

	if ( y != lasty ) {
		int nblines = mBuffer->lineCount();
		viewInformation.percentage = QString("%1%").arg( ( y*100/ ( nblines==0 ? 1 : nblines )));
		if ( scrollCursor->bufferY() < 1 )  viewInformation.percentage=_( "Top" );
		if ( scrollCursor->bufferY()+mLinesVis >= nblines )  viewInformation.percentage=_( "Bot" );
		if ( (scrollCursor->bufferY()<1 ) &&  ( scrollCursor->bufferY()+mLinesVis >= nblines ) ) viewInformation.percentage=_( "All" );
		lasty=y;
	}

	viewInformation.l = y;
	viewInformation.c1 = mainCursor->bufferX();
	viewInformation.c2 = mainCursor->screenX(); // XXX pas du tout, c'est c1 mais en remplacant les tabs par 'tablenght' <-- avec le QRegexp() mais je l'ai perdu

	syncViewInfo();
}

void YZView::centerViewHorizontally( int column) {
//	yzDebug() << "YZView::centerViewHorizontally " << column << endl;
	int newcurrentLeft = 0;
	if ( column > mColumnsVis/2 ) newcurrentLeft = column - mColumnsVis / 2;
	if ( newcurrentLeft != scrollCursor->bufferX() ) {
		// we are not in wrap mode, so buffer == screen
		scrollCursor->setBufferX( newcurrentLeft );
		scrollCursor->setScreenX( newcurrentLeft );
		sendRefreshEvent();
	}
}

void YZView::centerViewVertically( int line ) {
	if ( line == -1 )
		line = mainCursor->screenY();
	int newcurrent = 0;
	if ( line > mLinesVis / 2 ) newcurrent = line - mLinesVis / 2;
	alignViewVertically ( newcurrent );
}

void YZView::bottomViewVertically( int line ) {
	int newcurrent = 0;
	if ( line >= mLinesVis ) newcurrent = (line - mLinesVis) + 1;
	alignViewVertically( newcurrent );
}

void YZView::alignViewBufferVertically( int line ) {
//	yzDebug() << "YZView::alignViewBufferVertically " << line << endl;
	int newcurrent = line;
	int old_dCurrentTop = scrollCursor->screenY();
	if ( newcurrent > 0 ) {
		if ( wrap ) {
			gotodxy( scrollCursor, scrollCursor->screenX(), newcurrent );
		} else {
			scrollCursor->setBufferY( newcurrent );
			scrollCursor->setScreenY( newcurrent );
		}
	} else {
		scrollCursor->reset();
	}
	if ( old_dCurrentTop == scrollCursor->screenY() ) {
		/* nothing has changed */
		return;
	} else if ( qAbs(old_dCurrentTop - scrollCursor->screenY()) < mLinesVis ) {
		/* optimisation: we can scroll */
		internalScroll( 0, old_dCurrentTop - scrollCursor->screenY() );
	} else {
		sendRefreshEvent();
	}

	// find the last visible line in the buffer
	int lastBufferLineVisible = getCurrentTop() + getLinesVisible() - 1;

	if ( wrap ) {
		YZViewCursor temp = *scrollCursor;
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

void YZView::alignViewVertically( int line ) {
//	yzDebug() << "YZView::alignViewVertically " << line << endl;
	int newcurrent = line;
	int screenX = scrollCursor->screenX();
	int old_dCurrentTop = scrollCursor->screenY();
	if ( newcurrent > 0 ) {
		if ( wrap ) {
			initGoto( scrollCursor );
			gotody( newcurrent );
			// rLineHeight > 1 => our new top is in middle of a wrapped line, move new top to next line
			newcurrent = workCursor->bufferY();
			if ( workCursor->lineHeight > 1 ) 
				++newcurrent;
			gotoy( newcurrent );
			gotodx( screenX );
			applyGoto( scrollCursor, false );
		} else {
			scrollCursor->setBufferY( newcurrent );
			scrollCursor->setScreenY( newcurrent );
		}
	} else {
		scrollCursor->reset();
	}

	if ( old_dCurrentTop == scrollCursor->screenY() ) {
		/* nothing has changed */
		return;
	} else if ( qAbs(old_dCurrentTop - scrollCursor->screenY()) < mLinesVis ) {
		/* optimisation: we can scroll */
		internalScroll( 0, old_dCurrentTop - scrollCursor->screenY() );
	} else {
		sendRefreshEvent();
	}
}

/*
 * all the goto-like commands
 */

/* PRIVATE */
void YZView::gotodx( int nextx ) {
	YZASSERT( nextx >= 0 );
	YZIS_SAFE_MODE {
		if ( nextx < 0 ) nextx = 0;
	}
	int shift = !drawMode && mModePool->current()->isEditMode() && sCurLineLength > 0 ? 0 : 1;
	if ( sCurLineLength == 0 ) nextx = 0;

	// WARNING! we must drawPrevCol _before_ drawNextCol because drawPrevCol doesn't support tab!
	while ( workCursor->screenX() > nextx )
		if ( ! drawPrevCol( ) ) break;
	YZViewCursor last = *workCursor;
	while ( workCursor->screenX() < nextx && workCursor->bufferX() < sCurLineLength - shift ) {
		last = *workCursor;
		drawNextCol( );
	}
	if ( workCursor->screenX() > nextx )
		*workCursor = last;
}

void YZView::gotox( int nextx, bool forceGoBehindEOL ) {
	YZASSERT( nextx >= 0 );
	YZIS_SAFE_MODE {
		if ( nextx < 0 ) nextx = 0;
	}
	int shift = (!drawMode && mModePool->current()->isEditMode() && sCurLineLength > 0) || forceGoBehindEOL ? 1 : 0;
	if ( nextx >= sCurLineLength ) {
		if ( sCurLineLength == 0 ) nextx = 0;
		else nextx = sCurLineLength - 1 + shift;
	}
	while ( workCursor->bufferX() > nextx ) {
		if ( ! wrap || rCurLineLength <= mColumnsVis - shift ) {
			if ( ! drawPrevCol( ) ) break;
		} else {
			if ( ! drawPrevCol( ) ) {
				if ( workCursor->bufferX() >= nextx && workCursor->wrapNextLine ) drawPrevLine( );
				else break;
			}
		}
	}
	bool maywrap = wrap && rCurLineLength + shift > mColumnsVis;
	while ( workCursor->bufferX() < nextx ) {
		if ( !maywrap ) {
			drawNextCol( );
		} else {
			while ( drawNextCol() && workCursor->bufferX() < nextx ) ;
			if ( workCursor->wrapNextLine ) { 
				drawNextLine();
				maywrap = rCurLineLength + shift > mColumnsVis;
			} else if ( shift && workCursor->bufferX() == nextx && workCursor->screenX() == mColumnsVis ) {
				workCursor->wrapNextLine = true;
				drawNextLine();
				maywrap = rCurLineLength + shift > mColumnsVis;
			}
		}
	}
}

void YZView::gotody( int nexty ) {
	YZASSERT( nexty >= 0 );
	YZIS_SAFE_MODE {
		if ( nexty < 0 ) nexty = 0;
	}
	if ( workCursor->bufferY() >= mBuffer->lineCount() ) nexty = qMax( 0, mBuffer->lineCount() - 1 );

	/* some easy case */
	if ( nexty == 0 ) {
		gotoy( 0 );
	} else if ( nexty == scrollCursor->screenY() ) {
		gotoy( scrollCursor->bufferY() );
	} else {
		/** gotody when cursor is > nexty seems buggy, use gotoy way, I'll try to find a better solution */
		bool first = true;
		while ( workCursor->screenY() > nexty ) {
			if ( first && wrap && rCurLineLength > mColumnsVis ) { // move to begin of line
				initDraw( 0, workCursor->bufferY(), 0, workCursor->screenY() - workCursor->lineHeight + 1, drawMode );
				workCursor->lineHeight = workCursor->sLineIncrement = workCursor->bLineIncrement = 1;
				first = false;
			}
			drawPrevLine( );
			if ( wrap && rCurLineLength > mColumnsVis ) {
				/* goto begin of line */
				int wrapLineMinHeight = (int)ceil( rMinCurLineLength / mColumnsVis ) + 1;
				int wrapLineMaxHeight = (int)ceil( rCurLineLength / mColumnsVis ) + 1;
				if ( wrapLineMinHeight == wrapLineMaxHeight ) {
					workCursor->setScreenY( workCursor->screenY() + 1 - wrapLineMinHeight );
				} else {
					int cury = workCursor->bufferY();
					int prevRX = workCursor->screenY();
					initDraw( 0, cury, 0, 0, drawMode );
					while ( drawNextCol( ) ) ;
					while ( workCursor->bufferY() == cury ) {
						wrapLineMinHeight = workCursor->lineHeight;
						drawNextLine( );
						if ( workCursor->bufferY() == cury ) while ( drawNextCol( ) ) ;
					}
					initDraw ( 0, cury, 0, prevRX - wrapLineMinHeight + 1, drawMode );
					workCursor->lineHeight = workCursor->sLineIncrement = workCursor->bLineIncrement = 1;
				}
			}
		}
		while ( workCursor->screenY() < nexty && workCursor->bufferY() < mBuffer->lineCount() - 1 ) {
			if ( wrap && ! workCursor->wrapNextLine && rCurLineLength > mColumnsVis ) // make line wrapping
				while( drawNextCol( ) ) ;
			drawNextLine( );
			if ( wrap && workCursor->screenY() < nexty && rCurLineLength > mColumnsVis ) // move to end of draw line
				while ( drawNextCol( ) ) ;
		}
	}
}

void YZView::gotoy( int nexty ) {
	YZASSERT( nexty >= 0 );
	YZIS_SAFE_MODE {
		if ( nexty < 0 ) nexty = 0;
	}
	if ( nexty >= mBuffer->lineCount() ) nexty = qMax( 0, mBuffer->lineCount() - 1 );

	/* some easy case */
	if ( nexty == 0 ) {
		initDraw( 0, 0, 0, 0, drawMode );
		workCursor->lineHeight = workCursor->sLineIncrement = workCursor->bLineIncrement = 1;
	} else if ( nexty == scrollCursor->bufferY() ) {
		bool old_drawMode = drawMode;
		initDraw( ); // XXX
		drawMode = old_drawMode;
		workCursor->lineHeight = workCursor->sLineIncrement = workCursor->bLineIncrement = 1;
	} else if ( nexty != workCursor->bufferY() ) {
		bool first = true;
		while ( workCursor->bufferY() > nexty ) {
			if ( first && wrap && rCurLineLength > mColumnsVis ) { // move to begin of line
				initDraw( 0, workCursor->bufferY(), 0, workCursor->screenY() - workCursor->lineHeight + 1, drawMode );
				workCursor->lineHeight = workCursor->sLineIncrement = workCursor->bLineIncrement = 1;
				first = false;
			}
			drawPrevLine( );
			if ( wrap && rCurLineLength > mColumnsVis ) {
				/* goto begin of line */
				int wrapLineMinHeight = (int)ceil( rMinCurLineLength / mColumnsVis ) + 1;
				int wrapLineMaxHeight = (int)ceil( rCurLineLength / mColumnsVis ) + 1;
				if ( wrapLineMinHeight == wrapLineMaxHeight ) {
					workCursor->setScreenY( workCursor->screenY() + 1 - wrapLineMinHeight );
				} else {
					int cury = workCursor->bufferY();
					int prevRX = workCursor->screenY();
					initDraw( 0, cury, 0, 0, drawMode );
					while ( drawNextCol( ) ) ;
					while ( workCursor->bufferY() == cury ) {
						wrapLineMinHeight = workCursor->lineHeight;
						drawNextLine( );
						if ( workCursor->bufferY() == cury ) while ( drawNextCol( ) ) ;
					}
					initDraw ( 0, cury, 0, prevRX - wrapLineMinHeight + 1, drawMode );
					workCursor->lineHeight = workCursor->sLineIncrement = workCursor->bLineIncrement = 1;
				}
			}
		}
		while ( workCursor->bufferY() < nexty ) {
			if ( wrap && ! workCursor->wrapNextLine && rCurLineLength > mColumnsVis ) // move to end of draw line
				while ( drawNextCol( ) );
			drawNextLine( );
			if ( wrap && workCursor->bufferY() < nexty && rCurLineLength > mColumnsVis ) // move to end of draw line
				while ( drawNextCol( ) ) ;
		}
	}
}

void YZView::initGoto( YZViewCursor* viewCursor ) {
	initDraw( viewCursor->bufferX(), viewCursor->bufferY(), viewCursor->screenX(), viewCursor->screenY(), false );
	*workCursor = *viewCursor;
}

void YZView::applyGoto( YZViewCursor* viewCursor, bool applyCursor ) {
	*viewCursor = *workCursor;

	if ( applyCursor && viewCursor != mainCursor ) { // do not apply if this isn't the mainCursor
//		yzDebug() << "THIS IS NOT THE MAINCURSOR" << endl;
		applyCursor = false;
	} else if ( applyCursor && m_paintAutoCommit > 0 ) {
		sendCursor( viewCursor );
		applyCursor = false;
	}
	

/*	yzDebug() << "applyGoto : "
			<< "dColLength=" << dColLength << "; dLineLength=" << dLineLength << "; mLineLength=" << mLineLength
			<< "; dWrapNextLine=" << dWrapNextLine << "; dWrapTab=" << dWrapTab << endl;
	yzDebug() << "mCursor:" << *mCursor << "; dCursor:" << *dCursor << endl; */

	if ( applyCursor ) {

		setPaintAutoCommit( false );

		mModePool->current()->cursorMoved( this );

		if ( !isColumnVisible( mainCursor->screenX(), mainCursor->screenY() ) ) {
			centerViewHorizontally( mainCursor->screenX( ) );
		}
		if ( !isLineVisible( mainCursor->screenY() ) ) {
			if ( mainCursor->screenY() >= mLinesVis + scrollCursor->screenY() )
				bottomViewVertically( mainCursor->screenY() );
			else
				alignViewVertically( mainCursor->screenY() );
		}
		commitPaintEvent();
		updateCursor( );
	}
}


/* goto xdraw, ydraw */
void YZView::gotodxdy( int nextx, int nexty, bool applyCursor ) {
	gotodxdy( mainCursor, nextx, nexty, applyCursor );
}
void YZView::gotodxdy( YZViewCursor* viewCursor, int nextx, int nexty, bool applyCursor ) {
	initGoto( viewCursor );
	gotody( nexty );
	gotodx( nextx );
	applyGoto( viewCursor, applyCursor );
}

/* goto xdraw, ybuffer */
void YZView::gotodxy( int nextx, int nexty, bool applyCursor ) {
	gotodxy( mainCursor, nextx, nexty, applyCursor );
}
void YZView::gotodxy( YZViewCursor* viewCursor, int nextx, int nexty, bool applyCursor ) {
	initGoto( viewCursor );
	gotoy( mFoldPool->lineHeadingFold( nexty ) );
	gotodx( nextx );
	applyGoto( viewCursor, applyCursor );
}

/* goto xdraw, ybuffer */
void YZView::gotoxdy( int nextx, int nexty, bool applyCursor ) {
	gotoxdy( mainCursor, nextx, nexty, applyCursor );
}
void YZView::gotoxdy( YZViewCursor* viewCursor, int nextx, int nexty, bool applyCursor ) {
	initGoto( viewCursor );
	gotody( nexty );
	gotox( nextx );
	applyGoto( viewCursor, applyCursor );
}

/* goto xbuffer, ybuffer */
void YZView::gotoxy(int nextx, int nexty, bool applyCursor ) {
	gotoxy( mainCursor, nextx, nexty, applyCursor );
}
void YZView::gotoxy( YZViewCursor* viewCursor, int nextx, int nexty, bool applyCursor ) {
	initGoto( viewCursor );
	gotoy( mFoldPool->lineHeadingFold( nexty ) );
	gotox( nextx, viewCursor != mainCursor );
	applyGoto( viewCursor, applyCursor );
}

void YZView::gotodxdyAndStick( const YZCursor& pos ) {
	gotodxdy( mainCursor, pos.x(), pos.y(), true );
	updateStickyCol( mainCursor );
}
void YZView::gotodxdyAndStick( int x, int y ) {
	gotodxdyAndStick( YZCursor(x,y) );
}

void YZView::gotoxyAndStick( const YZCursor& pos ) {
	gotoxy( mainCursor, pos.x(), pos.y() );
	updateStickyCol( mainCursor );
}
void YZView::gotoxyAndStick( int x, int y ) {
	gotoxyAndStick( YZCursor(x,y) );
}

QString YZView::moveDown( int nb_lines, bool applyCursor ) {
	return moveDown( mainCursor, nb_lines, applyCursor );
}
QString YZView::moveDown( YZViewCursor* viewCursor, int nb_lines, bool applyCursor ) {
	gotoStickyCol( viewCursor, qMin( mFoldPool->lineAfterFold( viewCursor->bufferY() + nb_lines ), mBuffer->lineCount() - 1 ), applyCursor );
	return QString::null;
}
QString YZView::moveUp( int nb_lines, bool applyCursor ) {
	return moveUp( mainCursor, nb_lines, applyCursor );
}
QString YZView::moveUp( YZViewCursor* viewCursor, int nb_lines, bool applyCursor ) {
	gotoStickyCol( viewCursor, qMax( viewCursor->bufferY() - nb_lines, 0 ), applyCursor );
	return QString::null;
}

QString YZView::moveLeft( int nb_cols, bool wrap, bool applyCursor ) {
	return moveLeft( mainCursor, nb_cols, wrap, applyCursor );
}

QString YZView::moveLeft( YZViewCursor* viewCursor, int nb_cols, bool wrap, bool applyCursor ) {
	int x=int(viewCursor->bufferX());
	int y=viewCursor->bufferY();
	x-=nb_cols;
	if(x<0) {
		if(wrap) {
			int line_length;
			int diff=-x; // the number of columns we moved too far
			x=0;
			while(diff>0 && y>=1) {
				// go one line up
				line_length = myBuffer()->textline(--y).length();
				yzDebug() << "line length: " << line_length << endl;
				diff-=line_length+1;
			}
			// if we moved too far, go back
			if(diff < 0) x-=diff;
		} else
			x=0;
	}
	gotoxy( viewCursor, x, y);

	if ( applyCursor ) updateStickyCol( viewCursor );

	//return something
	return QString::null;
}

QString YZView::moveRight( int nb_cols, bool wrap, bool applyCursor ) {
	return moveRight( mainCursor, nb_cols, wrap, applyCursor );
}

QString YZView::moveRight( YZViewCursor* viewCursor, int nb_cols, bool wrap, bool applyCursor ) {
	int x=viewCursor->bufferX();
	int y=viewCursor->bufferY();
	x+=nb_cols;
	if(x>=myBuffer()->textline(y).length()) {
		if(wrap) {
			int line_length=myBuffer()->textline(y).length();
			int diff=x-line_length+1; // the number of columns we moved too far
			x=line_length-1;
			while(diff>0 && y<myBuffer()->lineCount()-1) {
				// go one line down
				line_length = myBuffer()->textline(++y).length();
				x=line_length-1;
				diff-=line_length+1;
			}
			// if we moved too far, go back
			if(diff < 0) x+=diff;
		} else
			x=myBuffer()->textline(y).length();
	}
	gotoxy( viewCursor, x, y);

	if ( applyCursor ) updateStickyCol( viewCursor );

	//return something
	return QString::null;
}

QString YZView::moveToFirstNonBlankOfLine( ) {
	return moveToFirstNonBlankOfLine( mainCursor );
}

QString YZView::moveToFirstNonBlankOfLine( YZViewCursor* viewCursor, bool applyCursor ) {
	//execute the code
	gotoxy( viewCursor, mBuffer->firstNonBlankChar(viewCursor->bufferY()) , viewCursor->bufferY(), applyCursor );
//	if ( viewCursor == mainCursor ) UPDATE_STICKY_COL;
	if ( applyCursor )
		updateStickyCol( viewCursor );

	//return something
	return QString::null;
}

QString YZView::moveToStartOfLine( ) {
	return moveToStartOfLine( mainCursor );
}

QString YZView::moveToStartOfLine( YZViewCursor* viewCursor, bool applyCursor ) {
	gotoxy(viewCursor, 0 , viewCursor->bufferY(), applyCursor);
	if ( applyCursor )
		updateStickyCol( viewCursor );

	return QString::null;
}

void YZView::gotoLastLine() {
	gotoLastLine( mainCursor );
}
void YZView::gotoLastLine( YZViewCursor* viewCursor, bool applyCursor ) {
	gotoLine( viewCursor, mBuffer->lineCount() - 1, applyCursor );
}
void YZView::gotoLine( int line ) {
	gotoLine( mainCursor, line );
}
void YZView::gotoLine( YZViewCursor* viewCursor, int line, bool applyCursor ) {
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

QString YZView::moveToEndOfLine( ) {
	return moveToEndOfLine( mainCursor );
}

QString YZView::moveToEndOfLine( YZViewCursor* viewCursor, bool applyCursor ) {
	gotoxy( viewCursor, mBuffer->textline( viewCursor->bufferY() ).length( ), viewCursor->bufferY(), applyCursor );
	if ( applyCursor )
		stickyCol = STICKY_COL_ENDLINE;

	return QString::null;
}

void YZView::applyStartPosition( const YZCursor& pos ) {
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
void YZView::initChanges( int x, int y ) {
	beginChanges->setX( x );
	beginChanges->setY( y );
	*origPos = mainCursor->buffer();
	lineDY = 1;
	if ( wrap && y < mBuffer->lineCount() ) {
		gotoxy( qMax( 1, mBuffer->getLineLength( y ) ) - 1, y, false );
		lineDY = mainCursor->screenY();
	}
	gotoxy( x, y, false );
}
void YZView::applyChanges( int /*x*/, int y ) {
	int dY = mainCursor->screenY() + 1 - mainCursor->lineHeight;
	if ( y != beginChanges->y() ) {
		sendPaintEvent( scrollCursor->screenX(), dY, mColumnsVis, mLinesVis - ( dY - scrollCursor->screenY() ) );
	} else {
		if ( wrap ) {
			gotoxy( qMax( 1, mBuffer->getLineLength( y ) ) - 1, y, false );
			if ( mainCursor->screenY() != lineDY )
				sendPaintEvent( scrollCursor->screenX(), dY, mColumnsVis, mLinesVis - ( dY - scrollCursor->screenY() ) );
			else
				sendPaintEvent( scrollCursor->screenX(), dY, mColumnsVis, 1 + mainCursor->screenY() - dY );
		} else
			sendPaintEvent( scrollCursor->screenX(), dY, mColumnsVis, 1 );
	}
	gotoxy( origPos->x(), origPos->y(), false );
}

QString YZView::append () {
	mModePool->change( YZMode::MODE_INSERT );
	gotoxyAndStick(mainCursor->bufferX()+1, mainCursor->bufferY() );
	return QString::null;
}

void YZView::commitUndoItem() {
	mBuffer->undoBuffer()->commitUndoItem(mainCursor->bufferX(), mainCursor->bufferY());
}

void YZView::pasteContent( QChar registr, bool after ) {
	QStringList list = YZSession::me->getRegister( registr );
	if ( list.isEmpty() ) return;

	YZCursor pos( mainCursor->buffer() );
	int i = 0;
	bool copyWholeLinesOnly = list[ 0 ].isNull();
	QString copy = mBuffer->textline( pos.y() );
	if ( after || ! copyWholeLinesOnly ) { //paste after current char
		int start;
		if( after )
			start = copy.length() > 0 ? pos.x() + 1 : 0;
		else
			start = pos.x();
		i = 0;
		if ( ! copyWholeLinesOnly ) {
			copy = copy.mid( start );
			mBuffer->action()->deleteChar( this, start, pos.y(), copy.length() );
			mBuffer->action()->insertChar( this, start, pos.y(), list[ 0 ] + ( list.size() == 1 ? copy : "" ) );
			gotoxy( start + list[ 0 ].length() - ( list[ 0 ].length() > 0 ? 1 : 0 ), pos.y() );
		}
		i++;
		while ( i < list.size() - 1 ) {
			mBuffer->action()->insertLine( this, pos.y() + i, list[ i ] );
			i++;
		}
		if ( i < list.size() && ! copyWholeLinesOnly ) {
			mBuffer->action()->insertLine( this, pos.y() + i, ( list[ i ].isNull() ? "" : list[ i ] ) + copy );
			gotoxy( list[ i ].length(), pos.y() + i );
		} else if ( copyWholeLinesOnly ) {
			gotoxy( 0, pos.y() + 1 );
			moveToFirstNonBlankOfLine();
		}

	} else if ( !after ) { //paste whole lines before current char
		for( i = 1; i < list.size() - 1; i++ )
			mBuffer->action()->insertLine( this, pos.y() + i - 1, list[ i ] );

		gotoxy( pos.x(), pos.y() );
	}
	updateStickyCol( mainCursor );
}

/*
 * Drawing engine
 */

void YZView::setFixedFont( bool fixed ) {
	isFontFixed = fixed;
	spaceWidth = GET_CHAR_WIDTH( ' ' );
}

bool YZView::isColumnVisible( int column, int  ) const {
	return ! (column < scrollCursor->screenX() || column >= (scrollCursor->screenX() + mColumnsVis));
}
bool YZView::isLineVisible( int l ) const {
	return ( ( l >= scrollCursor->screenY() ) && ( l < mLinesVis + scrollCursor->screenY() ) );
}
int YZView::getCurrentTop() const { 
	return scrollCursor->bufferY(); 
}
int YZView::getDrawCurrentTop() const {
	return scrollCursor->screenY();
}
int YZView::getCurrentLeft() const {
	return scrollCursor->bufferX();
}
int YZView::getDrawCurrentLeft() const {
	return scrollCursor->screenX();
}


/* update sCurLine informations */
void YZView::updateCurLine( ) {
	sCurLineLength = sCurLine.length();
	if ( wrap && ! drawMode ) {
		int nbTabs = sCurLine.count( '\t' );
		if ( isFontFixed ) rMinCurLineLength = sCurLineLength;
		else rMinCurLineLength = GET_STRING_WIDTH( QString( sCurLine ).remove( '\t' ) ) + nbTabs * spaceWidth;
		rCurLineLength = rMinCurLineLength + nbTabs * ( tablength - spaceWidth );
	}
}

int YZView::initDrawContents( int clipy ) {
	if ( ! wrap ) {
		initDraw( getCurrentLeft(), clipy, getDrawCurrentLeft(), clipy, true );
	} else {
		int currentY = getDrawCurrentTop();
		initDraw();
		drawMode = true;
		while( currentY < clipy && drawNextLine() ) {
			while( drawNextCol() );
			currentY += drawHeight();
		}
		clipy = currentY;
	}
	return clipy;
}

void YZView::initDraw( ) {
	initDraw( scrollCursor->bufferX(), scrollCursor->bufferY(), scrollCursor->screenX(), scrollCursor->screenY() );
}

/**
 * initDraw is called before each cursor/draw manipulation.
 */
void YZView::initDraw( int sLeft, int sTop, int rLeft, int rTop, bool draw ) {
	sCurrentLeft = sLeft;
	sCurrentTop = sTop;
	rCurrentLeft = rLeft;
	rCurrentTop = rTop;

	workCursor->setBufferX( sCurrentLeft );
	workCursor->setBufferY( sCurrentTop );
	workCursor->setScreenX( rCurrentLeft );
	workCursor->setScreenY( rCurrentTop );

	workCursor->sColIncrement = 1;
	workCursor->bLineIncrement = 0;
	workCursor->sLineIncrement = 0;

	workCursor->lineHeight = 1;

	workCursor->spaceFill = 0;

	adjust = false;

	tablength = tabstop * spaceWidth;
	areaModTab = ( tablength - mColumnsVis % tablength ) % tablength;

	workCursor->wrapNextLine = false;
	if ( workCursor->bufferY() < mBuffer->lineCount() ) {
		sCurLine = mBuffer->textline ( workCursor->bufferY() );
		if ( sCurLine.isNull() ) sCurLine = "";
	} else sCurLine = "";

	drawMode = draw;
	updateCurLine( );
}

bool YZView::drawPrevLine( ) {
	if ( ! workCursor->wrapNextLine ) {
		if ( workCursor->lineHeight > 1 ) {
			workCursor->sLineIncrement = 0;
			--workCursor->lineHeight;
		} else {
			workCursor->sLineIncrement = 1;
			workCursor->lineHeight = 1;
		}
		workCursor->setBufferX( sCurrentLeft );
		workCursor->setBufferY( mFoldPool->lineHeadingFold( workCursor->bufferY() - workCursor->sLineIncrement ) );
		workCursor->setScreenX( rCurrentLeft );
		if ( workCursor->sLineIncrement == 0 && workCursor->bLineIncrement > 0 ) {
			workCursor->sLineIncrement = 1;
		}
		workCursor->spaceFill = 0;
		workCursor->bLineIncrement = 1;
		workCursor->sColIncrement = 1;
	} else {
		workCursor->setScreenX( mColumnsVis - workCursor->sColIncrement );
		workCursor->spaceFill = workCursor->spaceFill - areaModTab;
		--workCursor->lineHeight;
	}
	workCursor->setScreenY( workCursor->screenY() - workCursor->sLineIncrement );
	workCursor->sLineIncrement = 1;

	if ( workCursor->bufferY() < mBuffer->lineCount() ) {
		if ( ! workCursor->wrapNextLine ) {
			sCurLine = mBuffer->textline( workCursor->bufferY() );
			updateCurLine( );
		}
		if ( rCurrentLeft > 0 && ! workCursor->wrapNextLine ) {
			workCursor->setScreenX( 0 );
			workCursor->setBufferX( 0 );
			gotodx( rCurrentLeft );
		}
		if ( ( workCursor->screenY() - rCurrentTop ) < mLinesVis ) {
			return true;
		}
	} else {
		sCurLine = "";
		sCurLineLength = sCurLine.length();
	}
	workCursor->wrapNextLine = false;
	return false;
}

bool YZView::drawNextLine( ) {
	if ( ! workCursor->wrapNextLine ) {
		workCursor->setBufferX( sCurrentLeft );
		workCursor->setBufferY( mFoldPool->lineAfterFold( workCursor->bufferY() + workCursor->bLineIncrement ) );
		workCursor->setScreenX( rCurrentLeft );
		if ( workCursor->sLineIncrement == 0 && workCursor->bLineIncrement > 0 ) {
			// this is need when drawNextCol is called before drawNextLine ( when scrolling )
			workCursor->sLineIncrement = 1;
		}
		workCursor->spaceFill = 0;
		workCursor->bLineIncrement = 1;
		workCursor->lineHeight = 1;
	} else {
		if ( workCursor->wrapTab ) workCursor->setBufferX( workCursor->bufferX() - 1 );
		workCursor->setScreenX( 0 );
		workCursor->spaceFill = ( workCursor->spaceFill + areaModTab ) % tablength;
		++workCursor->lineHeight;
		if ( workCursor->sLineIncrement == 0 ) {
			workCursor->sLineIncrement = 1;
		}
	}
	workCursor->setScreenY( workCursor->screenY() + workCursor->sLineIncrement );
	workCursor->sLineIncrement = 1;

	if ( workCursor->bufferY() < mBuffer->lineCount() ) {
		YZLine* yl = drawMode ? mBuffer->yzline( workCursor->bufferY(), false ) : NULL;
		if ( ! workCursor->wrapNextLine ) {
			sCurLine = drawMode ? yl->data() : mBuffer->textline( workCursor->bufferY() );
			updateCurLine( );
		}
		if ( rCurrentLeft > 0 && ! workCursor->wrapNextLine ) {
			workCursor->setBufferX( 0 );
			workCursor->setScreenX( 0 );
			adjust = true;
			gotodx( rCurrentLeft );
			adjust = false;
			if ( drawMode ) {
				if ( scrollCursor->bufferX() > 0 )
					workCursor->spaceFill = ( tablength - scrollCursor->bufferX() % tablength ) % tablength;
				if ( workCursor->screenX() > rCurrentLeft ) {
					workCursor->setBufferX( workCursor->bufferX() - 1 );
					workCursor->setScreenX( rCurrentLeft );
				}
			}
		}
		if ( drawMode && ( workCursor->screenY() - rCurrentTop ) < mLinesVis ) {
			m_lineFiller = ' ';
			m_lineMarker = ' ';
			if ( mFoldPool->isHead( workCursor->bufferY() ) ) {
				m_lineFiller = '-';
				m_lineMarker = '+';
			}
			rHLa = NULL;
			if ( yl->length() != 0 )
				rHLa = yl->attributes();
			rHLnoAttribs = !rHLa;
			rHLa = rHLa + workCursor->bufferX() - 1;
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
	workCursor->wrapNextLine = false;
	return false;
}

bool YZView::drawPrevCol( ) {
	workCursor->wrapNextLine = false;
	int shift = !drawMode && mModePool->current()->isEditMode() && sCurLineLength > 0 ? 1 : 0;
	if ( workCursor->bufferX() >= workCursor->bColIncrement ) {
		int curx = workCursor->bufferX() - 1;
		workCursor->setBufferX( curx );
		lastChar = sCurLine.at( curx );
		if ( lastChar != tabChar ) {
			workCursor->sColIncrement = GET_CHAR_WIDTH( lastChar );
			if ( workCursor->screenX() >= workCursor->sColIncrement )
				workCursor->setScreenX( workCursor->screenX() - workCursor->sColIncrement );
			else
				workCursor->wrapNextLine = ( wrap && rCurLineLength > mColumnsVis - shift && workCursor->screenX() == 0 && workCursor->bufferX() > 0 );
			workCursor->bLineIncrement = workCursor->wrapNextLine ? 0 : 1;
		} else {
			/* go back to begin of line */
			initDraw( 0, workCursor->bufferY(), 0, workCursor->screenY() - workCursor->lineHeight + 1, drawMode );
			return false;
		}
	}
	return ! workCursor->wrapNextLine;
}

bool YZView::drawNextCol( ) {
	bool ret = false;

	int curx = workCursor->bufferX();
	bool lastCharWasTab = workCursor->lastCharWasTab;

	int nextLength = ( drawMode ? 0 : spaceWidth );

	workCursor->sColIncrement = spaceWidth;
	workCursor->wrapNextLine = false;
	workCursor->lastCharWasTab = false;

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
			workCursor->sColIncrement = GET_CHAR_WIDTH( lastChar );
			lenToTest = workCursor->sColIncrement;
		} else {
			workCursor->lastCharWasTab = true;
			lastChar = ' ';
			listChar = drawMode && opt_list;
			if ( listChar ) {
				if ( opt_listchars["tab"].length() > 0 ) {
					lastChar = opt_listchars["tab"][0];
					if ( opt_listchars["tab"].length() > 1 )
						mFillChar = opt_listchars["tab"][1];
					if ( workCursor->wrapTab )
						lastChar = mFillChar;
				}
			}
			if ( workCursor->screenX( ) == scrollCursor->bufferX() )
				workCursor->sColIncrement = ( workCursor->spaceFill ? workCursor->spaceFill : tablength );
			else {
				// calculate tab position
				int mySpaceFill = ( scrollCursor->bufferX() == 0 ? workCursor->spaceFill : 0 );	// do not care about rSpaceFill if we arent't in wrapped mode
				if ( workCursor->screenX() >= mySpaceFill )
					workCursor->sColIncrement = ( ( workCursor->screenX() - mySpaceFill ) / tablength  + 1 ) * tablength + mySpaceFill - workCursor->screenX();
				else
					workCursor->sColIncrement = mySpaceFill + spaceWidth - workCursor->screenX();
			}
			if ( drawMode ) lenToTest = spaceWidth;
			else lenToTest = workCursor->sColIncrement;
		}
		if ( ! drawMode && ! isFontFixed && workCursor->bufferX() + workCursor->bColIncrement < sCurLineLength )
			nextLength = GET_CHAR_WIDTH( sCurLine.at( workCursor->bufferX() + workCursor->bColIncrement ) );

		// will our new char appear in the area ?
		ret = adjust || workCursor->screenX() + lenToTest - scrollCursor->screenX() <= mColumnsVis - nextLength;

		if ( ret || ! drawMode ) {
			// moving cursors
			workCursor->setScreenX( workCursor->screenX() + workCursor->sColIncrement );
			workCursor->setBufferX( workCursor->bufferX() + workCursor->bColIncrement );
			// update HL
			if ( drawMode ) rHLa += workCursor->bColIncrement;
		}
	} else if ( sCurLineLength == 0 && drawMode && curx == 0 ) { // empty line
		ret = true;
		lastChar = ' ';
		workCursor->setScreenX( 1 );
		workCursor->setBufferX( 1 );
	}

	if ( wrap ) {
		// screen pos
		int sx = workCursor->screenX() + nextLength + ( (ret || !drawMode) ? 0 : workCursor->sColIncrement );
		// buff pos
		int bx = curx + ( drawMode ? 0 : workCursor->bColIncrement );
		workCursor->wrapNextLine = sx > mColumnsVis;
		if ( bx == sCurLineLength ) { // wrap a tab at EOL
			workCursor->wrapNextLine &= ( drawMode ? lastCharWasTab : workCursor->lastCharWasTab );
		} else {
			workCursor->wrapNextLine &= bx < sCurLineLength;
		}
	}

	// only remember of case where wrapNextLine is true ( => we will wrap a tab next drawNextCol )
	workCursor->lastCharWasTab &= workCursor->wrapNextLine;

	// wrapNextLine is true, we are out of area ( ret is false ), last char was a tab => we are wrapping a tab
	workCursor->wrapTab = false;
	if ( workCursor->wrapNextLine ) {
		if ( drawMode )	{
			workCursor->wrapTab = ! ret && lastCharWasTab;
		} else {
			workCursor->wrapTab = workCursor->lastCharWasTab && workCursor->screenX() > mColumnsVis;
		}
	}
	// do not increment line buffer if we are wrapping a line
	workCursor->bLineIncrement = workCursor->wrapNextLine ? 0 : 1;

/*	if ( !drawMode && (workCursor->bufferY() == 12 || workCursor->bufferY() == 13 ) ) {
		workCursor->debug();
		yzDebug() << ret << endl;
	}
*/
	return ret;
}

const QChar& YZView::fillChar() const {
	return mFillChar;
}
const QChar& YZView::drawChar() const {
	return lastChar;
}
const QChar& YZView::drawLineFiller() const {
	return m_lineFiller;
}
const QChar& YZView::drawLineMarker() const {
	return m_lineMarker;
}
int YZView::drawLength() const {
	return workCursor->sColIncrement;
}
int YZView::drawHeight() const {
	return workCursor->sLineIncrement;
}
int YZView::lineIncrement() const {
	return workCursor->bLineIncrement;
}
int YZView::lineHeight() const {
	return workCursor->lineHeight;
}

const YZColor& YZView::drawColor ( int col, int line ) const {
	YZLine *yl = mBuffer->yzline( line );
	YzisHighlighting * highlight = mBuffer->highlight();
	const uchar* hl=NULL;
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

const YZColor& YZView::drawColor() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

	if ( listChar ) return blue; //XXX make custom
	else if ( curAt ) return curAt->textColor();
	else return color_null;
}

const YZColor& YZView::drawSelColor() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

	if ( listChar ) return color_null; //XXX make custom
	else if ( curAt ) return (*curAt).selectedTextColor();
	else return color_null;
}

const YZColor& YZView::drawBgColor() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

	if ( listChar ) return color_null; //XXX make custom
	else if ( curAt ) return (*curAt).bgColor();
	else return color_null;
}

const YZColor& YZView::drawBgSelColor() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

	if ( listChar ) return color_null; //XXX make custom
	else if ( curAt ) return (*curAt).selectedBGColor();
	else return color_null;
}

bool YZView::drawBold() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt )
		return (*curAt).bold();
	return false;	
}

bool YZView::drawItalic() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt )
		return (*curAt).italic();
	return false;	
}

bool YZView::drawUnderline() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt )
		return (*curAt).underline();
	return false;	
}

bool YZView::drawOverline() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt )
		return (*curAt).overline();
	return false;	
}

bool YZView::drawStrikeOutLine() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt )
		return (*curAt).strikeOut();
	return false;	
}

const YZColor& YZView::drawOutline() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ? &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

	if ( listChar ) return color_null; //XXX make custom
	else if ( curAt ) return (*curAt).outline();
	else return color_null;
}

int YZView::drawLineNumber() const {
	return workCursor->bufferY( ) + 1;
}

int YZView::drawTotalHeight() {
	int totalHeight = 0;
	int nb = mBuffer->lineCount();
	if ( nb > 0 ) {
		YZViewCursor cursor = viewCursor();
		int x = mBuffer->textline( nb - 1 ).length();
		if ( x > 0 ) --x;
		gotoxy( &cursor, x, nb - 1 );
		totalHeight = cursor.screenY() + 1;
	}
	return totalHeight;
}

void YZView::printToFile( const QString& /*path*/ ) {
#if 0
	if ( YZSession::getStringOption("printer") != "pslib" ) {
		if ( getenv( "DISPLAY" ) ) {
			YZQtPrinter qtprinter( this );
			qtprinter.printToFile( path );
			qtprinter.run( );
		} else {
			YZSession::me->popupMessage( _("To use the Qt printer, you need to have an X11 DISPLAY set and running, you should try pslib in console mode") );
		}
		return;
	}

#ifdef HAVE_LIBPS
	YZPrinter printer( this );
	printer.printToFile( path );
	printer.run( );
#endif
#endif
}

void YZView::undo( int count ) {
	for ( int i = 0 ; i < count ; i++ )
		mBuffer->undoBuffer()->undo( this );
}

void YZView::redo( int count ) {
	for ( int i = 0 ; i < count ; i++ )
		mBuffer->undoBuffer()->redo( this );
}


QString YZView::getLocalOptionKey() const {
	return mBuffer->fileName()+"-view-"+ QString::number(id.getNumber());
}
YZOptionValue* YZView::getLocalOption( const QString& option ) const {
	if ( YZSession::me->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) )//find the local one ?
		return YZSession::me->getOptions()->getOption( getLocalOptionKey() + "\\" + option );
	else
		return YZSession::me->getOptions()->getOption( "Global\\"+option );
}
int YZView::getLocalIntegerOption( const QString& option ) const {
	if ( YZSession::me->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) )//find the local one ?
		return YZSession::me->getOptions()->readIntegerOption( getLocalOptionKey()+ "\\" + option );
	else
		return YZSession::me->getOptions()->readIntegerOption( "Global\\" + option ); // else give the global default if any
}
bool YZView::getLocalBooleanOption( const QString& option ) const {
	if ( YZSession::me->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) )//find the local one ?
		return YZSession::me->getOptions()->readBooleanOption( getLocalOptionKey()+"\\"+option );
	else
		return YZSession::me->getOptions()->readBooleanOption( "Global\\" + option );
}
QString YZView::getLocalStringOption( const QString& option ) const {
	if ( YZSession::me->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) ) //find the local one ?
		return YZSession::me->getOptions()->readStringOption( getLocalOptionKey()+"\\"+option );
	else
		return YZSession::me->getOptions()->readStringOption( "Global\\" + option );
}
QStringList YZView::getLocalListOption( const QString& option ) const {
	if ( YZSession::me->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) ) //find the local one ?
		return YZSession::me->getOptions()->readListOption( getLocalOptionKey()+"\\"+option );
	else
		return YZSession::me->getOptions()->readListOption( "Global\\" + option );
}
MapOption YZView::getLocalMapOption( const QString& option ) const {
	if ( YZSession::me->getOptions()->hasOption( getLocalOptionKey() + "\\" + option ) ) //find the local one ?
		return YZSession::me->getOptions()->readMapOption( getLocalOptionKey()+"\\"+option );
	else
		return YZSession::me->getOptions()->readMapOption( "Global\\" + option );
}

void YZView::gotoStickyCol( int Y ) {
	gotoStickyCol( mainCursor, Y, true );
}

void YZView::gotoStickyCol( YZViewCursor* viewCursor, int Y, bool applyCursor ) {
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

QString YZView::getCharBelow( int delta ) {
	YZViewCursor vc = viewCursor();
	int Y = vc.bufferY();
	if ( delta < 0 && Y >= -delta || delta >= 0 && ( Y + delta ) < mBuffer->lineCount() )
		Y += delta;
	else
		return QString::null;

	QString ret = QString::null;
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

void YZView::updateStickyCol( ) {
	updateStickyCol( mainCursor );
}
void YZView::updateStickyCol( YZViewCursor* viewCursor ) {
	stickyCol = ( viewCursor->lineHeight - 1 ) * mColumnsVis + viewCursor->screenX();
}

void YZView::commitNextUndo() {
	mBuffer->undoBuffer()->commitUndoItem( mainCursor->bufferX(), mainCursor->bufferY() );
}

const YZCursor &YZView::getCursor() const {
	return mainCursor->screen();
}
const YZCursor &YZView::getBufferCursor() const {
	return mainCursor->buffer();
}
YZCursor YZView::getRelativeScreenCursor() const {
	return (QPoint)mainCursor->screen() - scrollCursor->screen();
}
YZCursor YZView::getScreenPosition() const {
	return scrollCursor->screen();
}

void YZView::recordMacro( const QList<QChar> &regs ) {
	mRegs = regs;
	for ( int ab = 0 ; ab < mRegs.size(); ++ab )
		YZSession::me->setRegister( mRegs.at(ab), QStringList());
}

void YZView::stopRecordMacro() {
	for ( int ab = 0 ; ab < mRegs.size(); ++ab ) {
		QStringList list;
		QString ne = YZSession::me->getRegister(mRegs.at(ab))[0];
		list << ne.mid( 0, ne.length() - 1 ); //remove the last 'q' which was recorded ;)
		YZSession::me->setRegister( mRegs.at(ab), list);
	}
	mRegs = QList<QChar>();
}

YZSelection YZView::clipSelection( const YZSelection& sel ) const {
	YZCursor bottomRight = scrollCursor->screen() + YZCursor(getColumnsVisible()-1,getLinesVisible()-1);
	return sel.clip( YZInterval(scrollCursor->screen(),bottomRight) );
}

void YZView::setPaintAutoCommit( bool enable ) {
	if ( enable ) {
		m_paintAutoCommit = 0;
	} else {
		++m_paintAutoCommit;
	}
}

void YZView::commitPaintEvent() {
	if ( m_paintAutoCommit == 0 ) return;
	if ( --m_paintAutoCommit == 0 ) {
		if ( keepCursor->valid() ) {
			*mainCursor = *keepCursor;
			keepCursor->invalidate();
			applyGoto( mainCursor );
		}
		if ( ! mPaintSelection->isEmpty() ) {
			YZCursor bottomRight = scrollCursor->screen();
			bottomRight.setX( bottomRight.x() + getColumnsVisible() );
			bottomRight.setY( bottomRight.y() + getLinesVisible() );
			notifyContentChanged( clipSelection(*mPaintSelection) );
		}
		abortPaintEvent();
	}
}
void YZView::abortPaintEvent() {
	keepCursor->invalidate();
	mPaintSelection->clear();
	setPaintAutoCommit();
}

void YZView::sendCursor( YZViewCursor* cursor ) {
	*keepCursor = *cursor;
}
void YZView::sendPaintEvent( const YZCursor& from, const YZCursor& to ) {
	m_paintAll = false;
	setPaintAutoCommit( false );
	mPaintSelection->addInterval( YZInterval( from, to ) );
	commitPaintEvent();
}
void YZView::sendPaintEvent( int curx, int cury, int curw, int curh ) {
	if ( curh == 0 ) {
		yzDebug() << "Warning: YZView::sendPaintEvent with height = 0" << endl;
		return;
	}
	sendPaintEvent( YZCursor(curx,cury),YZCursor(curx+curw,cury+curh-1) );
}
void YZView::sendPaintEvent( YZSelectionMap map, bool isBufferMap ) {
	int size = map.size();
	int i;
	if ( isBufferMap && wrap ) { // we must convert bufferMap to screenMap
		YZViewCursor vCursor = viewCursor();
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
void YZView::sendBufferPaintEvent( int line, int n ) {
	YZViewCursor vCursor = viewCursor();
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

void YZView::sendRefreshEvent( ) {
	mPaintSelection->clear();
	m_paintAll = true;
	sendPaintEvent( getDrawCurrentLeft(), getDrawCurrentTop(), getColumnsVisible(), getLinesVisible() );
}

void YZView::removePaintEvent( const YZCursor& from, const YZCursor& to ) {
	m_paintAll = false;
	mPaintSelection->delInterval( YZInterval( from, to ) );
}

bool YZView::stringHasOnlySpaces ( const QString& what ) {
	for (int i = 0 ; i < what.length(); i++)
		if ( !what.at(i).isSpace() ) {
			return false;
		}
	return true;
}

QString YZView::mode() {
	QString ret = mModePool->current()->toString();
	if ( isRecording() ) 
		ret += _(" { Recording }");
	return ret;
}

void YZView::saveInputBuffer() { 
	// We don't need to remember ENTER or ESC or CTRL-C
	if ( mPreviousChars == "<ENTER>" 
	||   mPreviousChars == "<ESC>" 
	||   mPreviousChars == "<CTRL>c" ) {
		return;
	}

	// If we are repeating the command don't overwrite
	if ( mPreviousChars != "." ) {
		mLastPreviousChars = mPreviousChars; 
	}
}

const YZViewId &YZView::getId() const
{
	return id;
}

QString YZView::getLineStatusString() const
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

int YZView::getSpaceWidth() const
{
	return spaceWidth;
}

void YZView::internalScroll( int dx, int dy ) {
	m_drawBuffer.scroll( dx, dy );
	scroll( dx, dy );
}

/**
 * default implementation for paintEvent
 */
void YZView::paintEvent( const YZSelection& drawMap ) {
	if ( drawMap.isEmpty() )
		return;
	//yzDebug() << "YZView::paintEvent" << drawMap;

	bool number = getLocalBooleanOption( "number" );
	if ( number ) {
		drawSetMaxLineNumber( myBuffer()->lineCount() );
	}
	if ( m_paintAll ) {
		/* entire screen has been updated already */
		return;
	}

	/* to calculate relative position */
	int shiftY = getDrawCurrentTop(); 
	int shiftX = getDrawCurrentLeft();
	int maxX = shiftX + getColumnsVisible();

	YZSelectionMap map = drawMap.map();
	int size = map.size();

	int fromY = map[ 0 ].fromPos().y(); /* first line */
	int toY = map[ size - 1 ].toPos().y(); /* last line */

	 /* where the draw begins */
	int curY = initDrawContents( fromY );
	int curX = 0;

	/* inform the view we want to paint from line <fromY> to <toY> */
	preparePaintEvent( curY - shiftY, toY - shiftY );

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

	bool interval_changed  = true;

	/* set selection layouts */
	m_drawBuffer.setSelectionLayout( YZSelectionPool::Search, *getSelectionPool()->search() - scrollCursor->screen() ); //XXX: search map is buffer only
	m_drawBuffer.setSelectionLayout( YZSelectionPool::Visual, getSelectionPool()->visual()->screen() - scrollCursor->screen() );

	while( curY <= toY && drawNextLine() ) {
		curX = shiftX;

		while( tY < curY ) { /* next interval */
			++mapIdx;
			fX = map[ mapIdx ].fromPos().x();
			fY = map[ mapIdx ].fromPos().y();
			tX = map[ mapIdx ].toPos().x();
			tY = map[ mapIdx ].toPos().y();
			interval_changed = true;
		}
		//yzDebug("painting") << curX << "," << curY << ":painting interval " << map[ mapIdx ] 
		//						<< " (changed=" << interval_changed << ")" << endl;
		if ( interval_changed ) {
			m_drawBuffer.replace( map[ mapIdx ] - scrollCursor->screen() );
			interval_changed = false;
		}

		drawStartAfterBOL = ( curY == fY && fX > shiftX );
		drawStopBeforeEOL = ( curY == tY && tX < maxX );

		drawLine = fY <= curY; // curY <= tY always true */
		drawIt = drawLine && !drawStartAfterBOL;
		drawEntireLine = drawIt && !drawStopBeforeEOL;

		clearToEOL = drawEntireLine || drawIt && curY != tY;

		if ( drawLine && number ) {
			drawSetLineNumber( curY - shiftY, drawLineNumber(), lineHeight() - 1 );
		}

		if ( drawIt && curY > fY ) {
			m_drawBuffer.newline( curY - shiftY );
		}
		while( drawNextCol() ) {
			if ( !drawEntireLine ) { /* we have to care of starting/stoping to draw on that line */
				if ( !drawIt && curY == fY ) { // start drawing ?
					drawIt = ( curX == fX );
					clearToEOL = drawIt && curY != tY;
				} else if ( drawIt && curY == tY ) { // stop drawing ?
					drawIt = !( curX > tX );
					if ( ! drawIt ) {
						++mapIdx;
						if ( mapIdx != size ) {
							fX = map[ mapIdx ].fromPos().x();
							fY = map[ mapIdx ].fromPos().y();
							tX = map[ mapIdx ].toPos().x();
							tY = map[ mapIdx ].toPos().y();
							m_drawBuffer.replace( map[ mapIdx ] - scrollCursor->screen() );
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
			drawClearToEOL( curX - shiftX, curY - shiftY, drawLineFiller() );
		}
		curY += drawHeight();
	}

	/* out of file lines (~) */
	int fh = shiftY + getLinesVisible();
	toY = qMin( toY, fh - 1 );
	m_drawBuffer.setColor( Qt::cyan );
	for( ; curY <= toY; ++curY ) {
		m_drawBuffer.newline( curY - shiftY );
		if ( number ) drawSetLineNumber( curY - shiftY, 0, 0 );
		m_drawBuffer.push( "~" );
		drawClearToEOL( 1, curY - shiftY, ' ' );
	}

	m_drawBuffer.flush();

//	yzDebug() << "after drawing: " << endl << m_drawBuffer << "--------" << endl;

	endPaintEvent();
}

