/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <marchand@kde.org>
 *  Thomas Capricelli <orzel@freehackers.org>.
 *  Loic Pauleve <panard@inzenet.org>
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

#include <cstdlib>
#include <ctype.h>
#include <qkeysequence.h>
#include <math.h>
#include "view.h"
#include "debug.h"
#include "undo.h"
#include "printer.h"
#include "cursor.h"
#include "options.h"
#include "registers.h"
#include "buffer.h"
#include "swapfile.h"
#include "mark.h"
#include "action.h"
#include "commands.h"
#include "motionpool.h"
#include "session.h"

#define STICKY_COL_ENDLINE -1

#define GOTO_STICKY_COL(Y) \
		if ( stickyCol == STICKY_COL_ENDLINE ) gotoxy( mBuffer->textline( Y ).length(), Y ); \
		else gotodxy( stickyCol, Y )
#define UPDATE_STICKY_COL	stickyCol = mainCursor->screenX()

#define GET_STRING_WIDTH( s ) ( isFontFixed ? s.length() : stringWidth( s ) )
#define GET_CHAR_WIDTH( c ) ( isFontFixed ? 1 : charWidth( c ) )

static const QChar tabChar( '\t' );
static QColor fake;

/**
 * class YZViewCursor
 */

YZViewCursor::YZViewCursor( YZView* parent ) {
	mParent = parent;
	mBuffer = new YZCursor( mParent );
	mScreen = new YZCursor( mParent );
	spaceFill = 0;
	bColIncrement = 1; // XXX seems to be no longer used... ( always 1 )
	bLineIncrement = 0;
	sColIncrement = 1;
	sLineIncrement = 0;
	lineHeight = 0;
	wrapNextLine = false;
	wrapTab = false;
}
YZViewCursor::~YZViewCursor( ) {
	delete mBuffer;
	delete mScreen;
}

/**
 * I didn't find better, we cannot do *to = *from cause of mBuffer and mScreen
 */
void YZViewCursor::copy( const YZViewCursor& orig ) {
	*mScreen = *orig.mScreen;
	*mBuffer = *orig.mBuffer;
	spaceFill = orig.spaceFill;
	bColIncrement = orig.bColIncrement;
	bLineIncrement = orig.bLineIncrement;
	sColIncrement = orig.sColIncrement;
	sLineIncrement = orig.sLineIncrement;
	lineHeight = orig.lineHeight;
	lastCharWasTab = orig.lastCharWasTab;
	wrapTab = orig.wrapTab;
	wrapNextLine = orig.wrapNextLine;
}

unsigned int YZViewCursor::bufferX() const {
	return mBuffer->getX();
}
unsigned int YZViewCursor::bufferY() const {
	return mBuffer->getY();
}
unsigned int YZViewCursor::screenX() const {
	return mScreen->getX();
}
unsigned int YZViewCursor::screenY() const {
	return mScreen->getY();
}

void YZViewCursor::setBuffer( const YZCursor& value ) {
	mBuffer->setCursor( value );
}
void YZViewCursor::setScreen( const YZCursor& value ) {
	mScreen->setCursor( value );
}
void YZViewCursor::setBufferX( unsigned int value ) {
	mBuffer->setX( value );
}
void YZViewCursor::setBufferY( unsigned int value ) {
	mBuffer->setY( value );
}
void YZViewCursor::setScreenX( unsigned int value ) {
	mScreen->setX( value );
}
void YZViewCursor::setScreenY( unsigned int value ) {
	mScreen->setY( value );
}


/**
 * class YZView
 */

YZView::YZView(YZBuffer *_b, YZSession *sess, int lines) {
	myId = YZSession::mNbViews++;
	yzDebug() << "New View created with UID : " << myId << endl;
	YZASSERT( _b ); YZASSERT( sess );
	mSession = sess;
	mBuffer	= _b;
	mLinesVis = lines;
	mColumnsVis = 0;

	mainCursor = new YZViewCursor( this );
	workCursor = new YZViewCursor( this );

	origPos = new YZCursor( this );

	/* start of visual mode */
	mVisualCursor = new YZCursor(this);
	dVisualCursor = new YZCursor(this);

	stickyCol = 0;

	mMode = YZ_VIEW_MODE_COMMAND;
	mCurrentLeft = 0;
	mCurrentTop = 0;
	dCurrentLeft = 0;
	dCurrentTop = 0;

	QString line = mBuffer->textline(mCurrentTop);

	mCurrentExItem = 0;
	mCurrentSearchItem = 0;
	mExHistory.resize(200);
	mSearchHistory.resize(200);
	reverseSearch=false;
	viewInformation.l = viewInformation.c1 = viewInformation.c2 = 0;
	viewInformation.percentage = "";
	mPreviousChars = "";

	selectionPool = new YZSelectionPool( this );
	setFixedFont( true );

	drawMode = false;
	rHLnoAttribs = false;
	rHLAttributesLen = 0;

	sCurLineLength = 0;
	rCurLineLength = 0;

	charSelected = false;

	lineDY = 0;
	tabstop = getLocalIntOption("tabstop");
	wrap = getLocalBoolOption( "wrap" );
}

YZView::~YZView() {
	yzDebug() << "YZView : Deleting view " << myId << endl;
	mBuffer->rmView(this); //make my buffer forget about me

	delete mainCursor;
	delete workCursor;
	delete origPos;
	delete mVisualCursor;
	delete dVisualCursor;
	delete selectionPool;
}

void YZView::setVisibleArea(int c, int l, bool refresh) {
	yzDebug() << "YZView::setVisibleArea(" << c << "," << l << ");" << endl;
	mLinesVis = l;
	mColumnsVis = c;
	if( refresh ) {
		refreshScreen();
		gotoxy( mainCursor->bufferX(), mainCursor->bufferY() );
	}
}

QString YZView::buildCommand( const QString& key, int modifiers ) {
	QString command=mPreviousChars;
	if ( modifiers & Qt::ControlButton ) command += "<CTRL>";
	if ( modifiers & Qt::ShiftButton ) command += "<SHIFT>";
	if ( modifiers & Qt::AltButton ) command += "<ALT>";
	if ( modifiers & Qt::MetaButton ) command += "<META>";
	command += key;

	return command;
}

void YZView::sendKey( const QString& key, const QString& modifiers) {
	//TODO swapfile
	if ( mBuffer->introShown() ) {
		mBuffer->clearIntro();
		gotoxy( 0,0 );
		return;
	}

	switch(mMode) {
		case YZ_VIEW_MODE_INSERT:
		case YZ_VIEW_MODE_REPLACE:
		case YZ_VIEW_MODE_SEARCH:
		case YZ_VIEW_MODE_EX:
		case YZ_VIEW_MODE_VISUAL:
		case YZ_VIEW_MODE_VISUAL_LINE :
		case YZ_VIEW_MODE_COMMAND:
		case YZ_VIEW_MODE_OPEN:
			;
	}
}

/* Used by the buffer to post events */
void YZView::sendKey( int c, int modifiers) {
	mBuffer->getSwapFile()->addToSwap( c, modifiers );
	if ( mBuffer->introShown() ) {
		mBuffer->clearIntro();
		gotoxy( 0,0 );
		return;
	}

	QString lin;
	QString key = QChar( tolower(c) );// = QKeySequence( c );
	//default is lower case unless some modifiers
	if ( modifiers & Qt::ShiftButton )
		key = key.upper();
	
	bool test = false;

	QString mappedCommand = buildCommand(key, modifiers);
//	yzDebug() << "MappedCommand " << mappedCommand << endl;

	//ignore some keys
	if ( c == Qt::Key_Shift || c == Qt::Key_Alt || c == Qt::Key_Meta ||c == Qt::Key_Control || c == Qt::Key_CapsLock ) {
		yzError( )<< "receiving modifiers in c, shouldn't happen" <<endl;
		return;
	}

	bool cindent = getLocalBoolOption( "cindent" );

	switch(mMode) {
		case YZ_VIEW_MODE_INSERT:
			switch ( c ) {
				case Qt::Key_Home:
					moveToStartOfLine( );
					return;
				case Qt::Key_End:
					moveToEndOfLine( );
					return;
				case Qt::Key_Insert:
					gotoReplaceMode( );
					return;
				case Qt::Key_Escape:
					if ( mainCursor->bufferX() > 0) moveLeft();
					gotoPreviousMode();
					return;
				case Qt::Key_Return: 
					{
						test = mainCursor->bufferX() == 0;
						QString currentLine = mBuffer->textline( mainCursor->bufferY() );
						if ( cindent && currentLine.simplifyWhiteSpace().endsWith( "{" ) ) {
							QRegExp rx("^(\\t*\\s*\\t*\\s*).*$");
							QString newline = "";
							if ( rx.exactMatch( currentLine ) )
								newline = rx.cap( 1 ); //that should have all tabs and spaces from the previous line
							newline.prepend( "\t" ); //add a TAB for the nextline
							mBuffer->action()->insertLine( this, YZCursor(this, 0, mainCursor->bufferY() + 1 ), newline );
							gotoxy( newline.length(), mainCursor->bufferY() );
						} else
							mBuffer->action()->insertNewLine( this, mainCursor->buffer() );
						if ( test ) {
							gotoxy( 0, mainCursor->bufferY() + 1 );
							UPDATE_STICKY_COL;
						}
						return;
					}
				case Qt::Key_Down:
					moveDown( );
					return;
				case Qt::Key_Left:
					moveLeft( );
					return;
				case Qt::Key_Right:
					moveRight( );
					return;
				case Qt::Key_Up:
					moveUp( );
					return;
				case Qt::Key_Backspace:
					if (mainCursor->bufferX() == 0 && mainCursor->bufferY() > 0 && getLocalStringOption( "backspace" ).contains( "eol" ) ) {
						joinLine( mainCursor->bufferY() - 1 );
						if ( mainCursor->bufferX() ) moveRight();
					} else if ( mainCursor->bufferX() > 0 )
						mBuffer->action()->deleteChar( this, mainCursor->bufferX() - 1, mainCursor->bufferY(), 1 );
					return;
				case Qt::Key_Delete:
					mBuffer->action()->deleteChar( this, mainCursor->buffer(), 1 );
					return;
				case Qt::Key_PageDown:
					GOTO_STICKY_COL( mainCursor->bufferY() + mLinesVis );
					purgeInputBuffer();
					return;
				case Qt::Key_PageUp:
					GOTO_STICKY_COL( mainCursor->bufferY() > mLinesVis ? mainCursor->bufferY() - mLinesVis : 0 );
					purgeInputBuffer();
					return;
				case Qt::Key_Tab:
					key = "\t";
				default:
					mBuffer->action()->insertChar( this, mainCursor->buffer(), key );
					if ( cindent && key == "}" )
						reindent(mainCursor->bufferX()-1, mainCursor->bufferY());
					return;
			}
			break;

		case YZ_VIEW_MODE_REPLACE:
			switch ( c ) {
				case Qt::Key_Home:
					moveToStartOfLine( );
					return;
				case Qt::Key_End:
					moveToEndOfLine( );
					return;
				case Qt::Key_Insert:
					gotoInsertMode( );
					return;
				case Qt::Key_Delete:
					mBuffer->action()->deleteChar( this, mainCursor->buffer(), 1 );
					return;
				case Qt::Key_Escape:
					if ( mainCursor->bufferX() == mBuffer->textline( mainCursor->bufferY() ).length() ) 
						moveToEndOfLine( );
					gotoPreviousMode();
					return;
				case Qt::Key_Return:
					test = mainCursor->bufferX() == 0;
					mBuffer->action()->insertNewLine( this, mainCursor->buffer() );
					if ( test ) {
						gotoxy( 0, mainCursor->bufferY() + 1 );
						UPDATE_STICKY_COL;
					}
					return;
				case Qt::Key_Down:
					moveDown( );
					return;
				case Qt::Key_Backspace:
				case Qt::Key_Left:
					moveLeft( );
					return;
				case Qt::Key_Right:
					moveRight( );
					return;
				case Qt::Key_Up:
					moveUp( );
					return;
				case Qt::Key_Tab:
					mBuffer->action()->replaceChar( this, mainCursor->buffer(), "\t" );
					return;
				case Qt::Key_PageDown:
					GOTO_STICKY_COL( mainCursor->bufferY() + mLinesVis );
					purgeInputBuffer();
					return;
				case Qt::Key_PageUp:
					GOTO_STICKY_COL( mainCursor->bufferY() > mLinesVis ? mainCursor->bufferY() - mLinesVis : 0 );
					purgeInputBuffer();
					return;
				default:
					mBuffer->action()->replaceChar( this, mainCursor->buffer(), key );
					return;
			}
			break;

		case YZ_VIEW_MODE_SEARCH:
			switch ( c ) {
				case Qt::Key_Return:
					yzDebug() << "Current search : " << getCommandLineText();
					if(getCommandLineText().isEmpty())
						return;
					mSearchHistory[mCurrentSearchItem] = getCommandLineText();
					mCurrentSearchItem++;
					doSearch( getCommandLineText() );
					setCommandLineText( "" );
					mSession->setFocusMainWindow();
					gotoPreviousMode();
					return;
				case Qt::Key_Down:
					if(mSearchHistory[mCurrentSearchItem].isEmpty())
						return;

					mCurrentSearchItem++;
					setCommandLineText( mSearchHistory[mCurrentSearchItem] );
					return;
				case Qt::Key_Left:
				case Qt::Key_Right:
					return;
				case Qt::Key_Up:
					if(mCurrentSearchItem == 0)
						return;

					mCurrentSearchItem--;
					setCommandLineText( mSearchHistory[mCurrentSearchItem] );
					return;
				case Qt::Key_Escape:
					setCommandLineText( "" );
					mSession->setFocusMainWindow();
					gotoPreviousMode();
					return;
				case Qt::Key_Backspace:
				{
					QString back = getCommandLineText();
					setCommandLineText(back.remove(back.length() - 1, 1));
					return;
				}
				default:
					setCommandLineText( getCommandLineText() + key );
					return;
			}

		case YZ_VIEW_MODE_EX:
			switch ( c ) {
				case Qt::Key_Return:{
					yzDebug() << "Current command EX : " << getCommandLineText();
					if(getCommandLineText().isEmpty())
						return;

					mExHistory[mCurrentExItem] = getCommandLineText();
					mCurrentExItem++;
					QString cmd = getCommandLineText();
					setCommandLineText( "" );
					mSession->setFocusMainWindow();
					gotoPreviousMode();
					mSession->getExPool()->execExCommand( this, cmd );
					return;}
				case Qt::Key_Down:
					if(mExHistory[mCurrentExItem].isEmpty())
						return;

					mCurrentExItem++;
					setCommandLineText( mExHistory[mCurrentExItem] );
					return;
				case Qt::Key_Left:
				case Qt::Key_Right:
					return;
				case Qt::Key_Up:
					if(mCurrentExItem == 0)
						return;

					mCurrentExItem--;
					setCommandLineText( mExHistory[mCurrentExItem] );
					return;
				case Qt::Key_Escape:
					setCommandLineText( "" );
					mSession->setFocusMainWindow();
					gotoPreviousMode();
					return;
				case Qt::Key_Tab:
					//ignore for now
					return;
				case Qt::Key_Backspace:
				{
					QString back = getCommandLineText();
					if ( back.isEmpty() ) {
						mSession->setFocusMainWindow();
						gotoPreviousMode();
						return;
						}
					setCommandLineText(back.remove(back.length() - 1, 1));
					return;
				}
				default:
					setCommandLineText( getCommandLineText() + key );
					return;
			}
			break;

		case YZ_VIEW_MODE_VISUAL:
		case YZ_VIEW_MODE_VISUAL_LINE :
			switch ( c ) {
				case Qt::Key_Escape:
					YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
					selectionPool->clear( "VISUAL" );
					paintEvent( dCurrentLeft, cur_sel.drawFrom().getY(), mColumnsVis, cur_sel.drawTo().getY() - cur_sel.drawFrom().getY() + 1 );
					purgeInputBuffer();
					gotoPreviousMode();
					return;
			}
		//dont break
		case YZ_VIEW_MODE_COMMAND:
			switch ( c ) {
				case Qt::Key_Home:
					key='0';
					break;
				case Qt::Key_End:
					key='$';
					break;
				case Qt::Key_Insert:
					key='i';
					break;
				case Qt::Key_Escape:
					purgeInputBuffer();
					return;
				case Qt::Key_Down:
					key='j';
					break;
				case Qt::Key_Left:
					key='h';
					break;
				case Qt::Key_Right:
					key='l';
					break;
				case Qt::Key_Up:
					key='k';
					break;
				case Qt::Key_Delete:
					if ( mMode == YZ_VIEW_MODE_VISUAL || mMode == YZ_VIEW_MODE_VISUAL_LINE ) {
						YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
						selectionPool->clear( "VISUAL" );
						mBuffer->action()->deleteArea( this, cur_sel.from(), cur_sel.to(), QChar( '\"' ));
						purgeInputBuffer();
						gotoCommandMode();
						return;
					} else
						mBuffer->action()->deleteChar( this, *mainCursor->buffer(), 1);
					return;
				case Qt::Key_PageDown:
					GOTO_STICKY_COL( mainCursor->bufferY() + mLinesVis );
					purgeInputBuffer();
					return;
				case Qt::Key_PageUp:
					GOTO_STICKY_COL( mainCursor->bufferY() > mLinesVis ? mainCursor->bufferY() - mLinesVis : 0 );
					purgeInputBuffer();
					return;
				default:
					break;
			}
			mPreviousChars+=key;
//			yzDebug() << "Previous chars : (" << int( (mPreviousChars.latin1())[0] )<< ") " << mPreviousChars << endl;
			if ( mSession ) {
				int error = 0;
				mSession->getPool()->execCommand(this, mappedCommand, &error);
				if ( error == 1 ) {
//					yzDebug("Commands") << "No matching command found at first pass" << endl;
					error = 0;
					mSession->getPool()->execCommand(this, mPreviousChars, &error); //try to find a command without the modifier
				} else
					break;
//				if ( error == 1 )
//					yzDebug("Commands") << "No matching command found at second pass. I give up..." << endl;
			}
			break;

		case YZ_VIEW_MODE_OPEN:
			switch (c) {
				case ':':
				case 'Q':
					gotoExMode();
					break;
				default:
					break;
			}
			break;
		default:
			yzDebug() << "Unknown MODE" << endl;
			purgeInputBuffer();
	};
}

void YZView::reindent( unsigned int X, unsigned int Y ) {
	yzDebug() << "Reindent " << endl;
	QRegExp rx("^(\\t*\\s*\\t*\\s*).*$"); //regexp to get all tabs and spaces
	QString currentLine = mBuffer->textline( Y ).stripWhiteSpace();
	bool found = false;
	YZCursor *cur = new YZCursor ( this, X, Y );
	YZCursor match = mBuffer->action()->match(this, *cur, &found);
	if ( !found ) return;
	yzDebug() << "Match found on line " << match.getY() << endl;
	QString matchLine = mBuffer->textline( match.getY() );
	if ( rx.exactMatch( matchLine ) )
		currentLine.prepend( rx.cap( 1 ) ); //that should have all tabs and spaces from the previous line
	YZCursor *c = new YZCursor(this, 0, mainCursor->bufferY());
	mBuffer->action()->replaceLine( this, c, currentLine );
	gotoxy( currentLine.length(), mainCursor->bufferY() );
}

void YZView::updateCursor() {
	static unsigned int lasty = 1<<31; // small speed optimisation
	viewInformation.percentage = tr( "All" );
	unsigned int y = mainCursor->bufferY();

	if ( y != lasty ) {
		unsigned int nblines = mBuffer->lineCount();
		viewInformation.percentage = QString("%1%").arg( ( unsigned int )( y*100/ ( nblines==0 ? 1 : nblines )));
		if ( mCurrentTop < 1 )  viewInformation.percentage=tr( "Top" );
		if ( mCurrentTop+mLinesVis >= nblines )  viewInformation.percentage=tr( "Bot" );
		if ( (mCurrentTop<1 ) &&  ( mCurrentTop+mLinesVis >= nblines ) ) viewInformation.percentage=tr( "All" );
		lasty=y;
	}

	viewInformation.l = y;
	viewInformation.c1 = mainCursor->bufferX(); 
	viewInformation.c2 = mainCursor->screenX(); // XXX pas du tout, c'est c1 mais en remplacant les tabs par 'tablenght' <-- avec le QRegexp() mais je l'ai perdu

	syncViewInfo();
}

void YZView::centerViewHorizontally(unsigned int column) {
//	yzDebug() << "YZView::centerViewHorizontally " << column << endl;
	unsigned int newcurrentLeft = 0;
	if ( column > mColumnsVis/2 ) newcurrentLeft = column - mColumnsVis / 2;

	if (newcurrentLeft > 0) {
		initGoto( );
		gotoy( mainCursor->bufferY() );
		gotodx( newcurrentLeft );
		mCurrentLeft = workCursor->bufferX();
		dCurrentLeft = workCursor->screenX();
		initDraw ( );
	} else {
		dCurrentLeft = 0;
		mCurrentLeft = 0;
	}
//	yzDebug() << "YZView::centerViewHorizontally : dCurrentLeft: " << dCurrentLeft << ", mCurrentLeft: " << mCurrentLeft << endl;
}

void YZView::centerViewVertically(unsigned int line) {
	unsigned int newcurrent = 0;
	if ( line > mLinesVis / 2 ) newcurrent = line - mLinesVis / 2;
	alignViewVertically ( newcurrent );
}

void YZView::alignViewVertically( unsigned int line ) {
//	yzDebug() << "YZView::alignViewVertically " << line << endl;
	unsigned int newcurrent = 0;
	bool alignTop = true;
	if ( line >= dCurrentTop + mLinesVis ) { 
		newcurrent = line - mLinesVis + 1;
		alignTop = false;
	} else if ( line > 0 ) newcurrent = line;
	unsigned int old_dCurrentTop = dCurrentTop;
//	yzDebug() << "newcurrent=" << newcurrent << "; alignTop=" << alignTop << "; old_dCurrentTop=" << dCurrentTop << endl;
	if ( wrap && newcurrent > 0 ) {
		initDraw();
		drawMode = false;
		gotody( newcurrent );
//		yzDebug() << "raw top = " << *sCursor << "; r=" << *rCursor << endl;
		// rLineHeight > 1 => our new top is in middle of a wrapped line, move new top to next line
		newcurrent = workCursor->bufferY() + ( !alignTop && workCursor->lineHeight > 1 ? 1 : 0 );
		gotoy( newcurrent );
		mCurrentTop = workCursor->bufferY();
		dCurrentTop = workCursor->screenY();
	} else {
		dCurrentTop = newcurrent;
		mCurrentTop = newcurrent;
	}
//	yzDebug() << "dCurrentTop = " << dCurrentTop << "; mCurrentTop=" << mCurrentTop << endl;
	if ( old_dCurrentTop > dCurrentTop && old_dCurrentTop - dCurrentTop < mLinesVis ) {
		scrollUp( old_dCurrentTop - dCurrentTop );
	} else if ( old_dCurrentTop < dCurrentTop && dCurrentTop - old_dCurrentTop < mLinesVis ) {
		scrollDown( dCurrentTop - old_dCurrentTop );
	} else {
		 refreshScreen();
	}
	if ( alignTop ) GOTO_STICKY_COL( mCurrentTop );
}

/* recalculate cursor position + refresh screen */
void YZView::reset( ) {
	initDraw( 0, 0, 0, 0 );
	drawMode = false;
	gotoy( mainCursor->bufferY() );
	gotox( mainCursor->bufferX() );
	applyGoto( );
	refreshScreen( );
}

/*
 * all the goto-like commands
 */

/* PRIVATE */
void YZView::gotodx( unsigned int nextx ) {
	if ( ( int )nextx < 0 ) nextx = 0;
	unsigned int shift = ( ! drawMode && ( YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT==mMode && sCurLineLength > 0 ) ) ? 0 : 1;
	if ( sCurLineLength == 0 ) nextx = 0;
	else if ( workCursor->bufferX() >= sCurLineLength ) {
		gotox ( sCurLineLength );
		return;
	}
	while ( workCursor->screenX() > nextx )
		if ( ! drawPrevCol( ) ) break;
	while ( workCursor->screenX() < nextx && workCursor->bufferX() < sCurLineLength - shift )
		drawNextCol( );
}

void YZView::gotox( unsigned int nextx ) {
	if ( ( int )nextx < 0 ) nextx = 0;
	unsigned int shift = ( ! drawMode && ( YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT==mMode && sCurLineLength > 0 ) ) ? 1 : 0;
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
	while ( workCursor->bufferX() < nextx ) {
		if ( ! wrap || rCurLineLength <= mColumnsVis - shift ) {
			drawNextCol( );
		} else {
			while ( drawNextCol() && workCursor->bufferX() < nextx ) ;
			if ( workCursor->wrapNextLine ) drawNextLine();
		}
	}
}

void YZView::gotody( unsigned int nexty ) {
	if ( ( int )nexty < 0 ) nexty = 0;
	if ( workCursor->bufferY() >= mBuffer->lineCount() ) nexty = mBuffer->lineCount() - 1;

	/* some easy case */
	if ( nexty == 0 ) {
		gotoy( 0 );
	} else if ( nexty == dCurrentTop ) {
		gotoy( mCurrentTop );
	} else {
		while ( workCursor->screenY() > nexty ) {
			drawPrevLine( );
		}
		while ( workCursor->screenY() < nexty && workCursor->bufferY() < mBuffer->lineCount() - 1 ) {
			if ( wrap && ! workCursor->wrapNextLine && rCurLineLength > mColumnsVis ) // make line wrapping
				while( drawNextCol( ) ) ;
			drawNextLine( );
			if ( wrap && workCursor->screenY() < nexty && rCurLineLength > mColumnsVis )
				while ( drawNextCol( ) ) ;
		}
	}
}

void YZView::gotoy( unsigned int nexty ) {
	if ( ( int )nexty < 0 ) nexty = 0;
	if ( nexty >= mBuffer->lineCount() ) nexty = mBuffer->lineCount() - 1;
	if ( nexty == workCursor->bufferY() ) return;

	/* some easy case */
	if ( nexty == 0 ) {
		initDraw( 0, 0, 0, 0, false ); 
		workCursor->lineHeight = workCursor->sLineIncrement = workCursor->bLineIncrement = 1;
	} else if ( nexty == mCurrentTop ) {
		initDraw( ); // XXX
		drawMode = false;
		workCursor->lineHeight = workCursor->sLineIncrement = workCursor->bLineIncrement = 1;
	} else {
		while ( workCursor->bufferY() > nexty ) {
			drawPrevLine( );
			if ( wrap && workCursor->bufferY() == nexty && rCurLineLength > mColumnsVis ) { 
				/* goto begin of line */
				unsigned int wrapLineMinHeight = ( unsigned int ) ceil( rMinCurLineLength / mColumnsVis ) + 1;
				unsigned int wrapLineMaxHeight = ( unsigned int ) ceil( rCurLineLength / mColumnsVis ) + 1;
				if ( wrapLineMinHeight == wrapLineMaxHeight ) {
					workCursor->setScreenY( workCursor->screenY() + 1 - wrapLineMinHeight );
				} else {
					unsigned int prevRX = workCursor->screenY();
					initDraw( 0, nexty, 0, 0, drawMode );
					while ( drawNextCol( ) ) ;
					while ( workCursor->bufferY() == nexty ) {
						wrapLineMinHeight = workCursor->lineHeight;
						drawNextLine( );
						if ( workCursor->bufferY() == nexty ) while ( drawNextCol( ) ) ;
					}
					initDraw ( 0, nexty, 0, prevRX - wrapLineMinHeight, drawMode );
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

void YZView::initGoto( ) {
	initDraw( mainCursor->bufferX(), mainCursor->bufferY(), mainCursor->screenX(), mainCursor->screenY(), false ); // XXX
	workCursor->copy( *mainCursor );
}

void YZView::applyGoto( bool applyCursor ) {
	mainCursor->copy( *workCursor );

/*	yzDebug() << "applyGoto : "
			<< "dColLength=" << dColLength << "; dLineLength=" << dLineLength << "; mLineLength=" << mLineLength
			<< "; dWrapNextLine=" << dWrapNextLine << "; dWrapTab=" << dWrapTab << endl; 
	yzDebug() << "mCursor:" << *mCursor << "; dCursor:" << *dCursor << endl; */

	if ( applyCursor ) {

		if ( mMode == YZ_VIEW_MODE_VISUAL || mMode == YZ_VIEW_MODE_VISUAL_LINE ) {

			YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
			/* erase current selection */
			selectionPool->clear( "VISUAL" );

			YZCursor bBegin( *mVisualCursor );
			YZCursor dBegin( *dVisualCursor );
			YZCursor bEnd( *mainCursor->buffer() );
			YZCursor dEnd( *mainCursor->screen() );

			if ( bBegin > bEnd ) {
				YZCursor bTmp( bEnd );
				YZCursor dTmp( dEnd );
				bEnd.setCursor( bBegin );
				dEnd.setCursor( dBegin );
				bBegin.setCursor( bTmp );
				dBegin.setCursor( dTmp );
			}
			if ( mMode == YZ_VIEW_MODE_VISUAL_LINE ) {
				bBegin.setX( 0 );
				bEnd.setX( mBuffer->textline( bEnd.getY() ).length() );
			}
			selectionPool->addSelection( "VISUAL", bBegin, bEnd, dBegin, dEnd );
			yzDebug() << "visual selection : from " << bBegin << " to " << bEnd << endl;

			/* apply new selection */
			YZSelection new_cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
			if ( new_cur_sel.drawFrom() > cur_sel.drawFrom() ) new_cur_sel.setDrawFrom(cur_sel.drawFrom() );
			if ( new_cur_sel.drawTo() < cur_sel.drawTo() ) new_cur_sel.setDrawTo ( cur_sel.drawTo() );
			paintEvent( dCurrentLeft, new_cur_sel.drawFrom().getY() > 0 ? new_cur_sel.drawFrom().getY() - 1 : 0, mColumnsVis, new_cur_sel.drawTo().getY() - new_cur_sel.drawFrom().getY() + 3 );
		}

		if ( !isLineVisible( mainCursor->screenY() ) )
			alignViewVertically( mainCursor->screenY() );
		if ( !isColumnVisible( mainCursor->screenX(), mainCursor->screenY() ) ) {
			centerViewHorizontally( mainCursor->screenX( ) );
			refreshScreen();
		}
		updateCursor( );
	}
}


/* goto xdraw, ydraw */
void YZView::gotodxdy( unsigned int nextx, unsigned int nexty, bool applyCursor ) {
	initGoto( );
	gotody( nexty );
	gotodx( nextx );
	applyGoto( applyCursor );
}

/* goto xdraw, ybuffer */
void YZView::gotodxy( unsigned int nextx, unsigned int nexty, bool applyCursor ) {
	initGoto( );
	gotoy( nexty );
	gotodx( nextx );
	applyGoto( applyCursor );
}

/* goto xdraw, ybuffer */
void YZView::gotoxdy( unsigned int nextx, unsigned int nexty, bool applyCursor ) {
	initGoto( );
	gotody( nexty );
	gotox( nextx );
	applyGoto( applyCursor );
}

/* goto xbuffer, ybuffer */
void YZView::gotoxy(unsigned int nextx, unsigned int nexty, bool applyCursor ) {
	initGoto( );
	gotoy( nexty );
	gotox( nextx );
	applyGoto( applyCursor );
}

QString YZView::moveDown( const QString& , YZCommandArgs args ) {
	unsigned int nb_lines=args.count;

	//execute the code
	unsigned int nextLine = QMIN( mainCursor->bufferY() + nb_lines, mBuffer->lineCount() - 1 );
	GOTO_STICKY_COL( nextLine );

	//reset the input buffer
	purgeInputBuffer();

	//return something
	return QString::null;
}

QString YZView::moveUp( const QString& , YZCommandArgs args ) {
	unsigned int nb_lines=args.count;

	//execute the code
	unsigned int nextLine = QMAX( mainCursor->bufferY() - nb_lines, 0 );
	GOTO_STICKY_COL( nextLine );

	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveLeft( const QString& , YZCommandArgs args ) {
	uint nb_cols=args.count;

	//execute the code
	unsigned int bX = mainCursor->bufferX();
	nb_cols = QMIN( bX, nb_cols );
	gotoxy( bX ? bX - nb_cols : 0 , mainCursor->bufferY());

	UPDATE_STICKY_COL;

	//reset the input buffer
	purgeInputBuffer();

	//return something
	return QString::null;
}

QString YZView::moveRight( const QString& , YZCommandArgs args ) {
	int nb_cols=args.count;
	
	//execute the code
	gotoxy( mainCursor->bufferX() + nb_cols , mainCursor->bufferY() );

	UPDATE_STICKY_COL;
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveToFirstNonBlankOfLine( const QString&, YZCommandArgs ) {
	//execute the code
	gotoxy(mBuffer->firstNonBlankChar(mainCursor->bufferY()) , mainCursor->bufferY());
	UPDATE_STICKY_COL;
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveToStartOfLine( const QString&, YZCommandArgs ) {
	//execute the code
	gotoxy(0 , mainCursor->bufferY());
	UPDATE_STICKY_COL;
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::gotoLine(const QString& inputsBuff, YZCommandArgs ) {
	// Accepts "gg", "G", "<number>gg", "<number>G"
	// "gg", "0gg", "1gg" -> first line of the file
	// "G", "0G", "3240909G", "23323gg" -> last line of the file
	// "<line>G", "<line>gg" -> line of the file
	// in gvim, there is a configuration, startofline, which tells whether
	// the cursor is set on the first non-blank character (default) or on the
	// same columns as it originally was

	uint line=0;

	// first and easy case to handle
	if ( inputsBuff.startsWith( "gg" ) ) {
		line = 1;
	} else if ( inputsBuff.startsWith( "G" ) ) {
		line = mBuffer->lineCount();
	} else {
		//try to find a number
		int i=0;
		while ( inputsBuff[i].isDigit() ) i++; //go on
		bool toint_ok;
		line = inputsBuff.left( i ).toInt( &toint_ok );
		if (!toint_ok || !line) {
			if (inputsBuff[i] == 'G') {
				line=mBuffer->lineCount();
			} else {
				line=1;
			}
		}
	}

	if ( line ) --line;
	if (line >= mBuffer->lineCount()) line = mBuffer->lineCount() - 1;

	if ( getLocalBoolOption("startofline") ) {
		gotoxy(mBuffer->firstNonBlankChar(line), line);
	} else {
		GOTO_STICKY_COL( line );
	}

	purgeInputBuffer();
	return QString::null; //return something
}

// end of goto-like command


QString YZView::moveToEndOfLine( const QString&, YZCommandArgs ) {
	gotoxy( mBuffer->textline( mainCursor->bufferY() ).length( ), mainCursor->bufferY());

	stickyCol = STICKY_COL_ENDLINE;
	
	purgeInputBuffer();
	return QString::null;
}

/*
 * ACTIONS
 */

void YZView::initChanges( const YZCursor& pos ) {
	origPos->setCursor( mainCursor->buffer() );
	if ( wrap ) {
		gotoxy( mBuffer->textline( pos.getY() ).length(), pos.getY(), false );
		lineDY = mainCursor->screenY();
	}
	gotoxy( pos.getX(), pos.getY(), false );
}

void YZView::applyChanges( const YZCursor& pos, unsigned int len, bool applyCursor ) {
	unsigned int dY = mainCursor->screenY();
	if ( wrap ) {
		gotoxy( mBuffer->textline( pos.getY() ).length(), pos.getY(), false );
		if ( mainCursor->screenY() != lineDY )
			paintEvent( dCurrentLeft, dY, mColumnsVis, mLinesVis - ( dY - dCurrentTop ) );
		else
			paintEvent( dCurrentLeft, dY, mColumnsVis, 1 + mainCursor->screenY() - dY );
	} else 
		paintEvent( dCurrentLeft, dY, mColumnsVis, 1 );

	if ( applyCursor )
		gotoxy( pos.getX() + len, pos.getY() );
	else 
		gotoxy( origPos->getX(), origPos->getY(), false );
	UPDATE_STICKY_COL;
}

void YZView::initInsertChar( const YZCursor& pos, unsigned int /*len*/, bool /*applyCursor*/ ) {
	initChanges( pos );
}

void YZView::applyInsertChar( const YZCursor& pos, unsigned int len, bool applyCursor ) {
	applyChanges( pos, len, applyCursor );
}

void YZView::initDeleteChar( const YZCursor& pos, unsigned int /*len*/, bool /*applyCursor*/ ) {
	initChanges( pos );
}

void YZView::applyDeleteChar( const YZCursor& pos, unsigned int /*len*/, bool applyCursor ) {
	applyChanges( pos, 0, applyCursor );
}

void YZView::initReplaceChar( const YZCursor& pos, unsigned int /*len*/, bool /*applyCursor*/ ) {
	initChanges( pos );
}

void YZView::applyReplaceChar( const YZCursor& pos, unsigned int len, bool applyCursor ) {
	applyChanges( pos, len, applyCursor );
}

void YZView::initReplaceLine( const YZCursor& pos, bool /*applyCursor*/ ) {
	initChanges( pos );
}

void YZView::applyReplaceLine( const YZCursor& pos, unsigned int len, bool applyCursor ) {
	applyChanges( pos, len, applyCursor );
}

void YZView::initInsertLine( const YZCursor& pos, bool /*applyCursor*/ ) {
	origPos->setCursor( mainCursor->buffer() );
	gotoxy( pos.getX(), pos.getY(), false );
}

void YZView::applyInsertLine( const YZCursor& pos, bool applyCursor ) { 
	paintEvent( dCurrentLeft, mainCursor->screenY(), mColumnsVis, mLinesVis - ( mainCursor->screenY() - dCurrentTop ) );
	if ( applyCursor )
		gotoxy( 0, pos.getY() + ( pos.getX() ? 1 : 0 ) );
	else
		gotoxy( origPos->getX(), origPos->getY() );
	UPDATE_STICKY_COL;
}

void YZView::initDeleteLine( const YZCursor& pos, unsigned int /*len*/, bool /*applyCursor*/ ) {
	origPos->setCursor( mainCursor->buffer() );
	gotoxy( 0, pos.getY() ? pos.getY() - 1 : pos.getY(), false );
}

void YZView::applyDeleteLine( const YZCursor& pos, unsigned int /*len*/, bool applyCursor ) {
	paintEvent( dCurrentLeft, mainCursor->screenY(), mColumnsVis, mLinesVis - ( mainCursor->screenY() - dCurrentTop ) );
	if ( applyCursor )
		GOTO_STICKY_COL( pos.getY() );
	else
		gotoxy( origPos->getX(), origPos->getY() );
	UPDATE_STICKY_COL;
}

void YZView::initDeleteLine( const YZCursor& begin, const YZCursor& end, bool /*applyCursor*/ ) {
	origPos->setCursor( mainCursor->buffer() );
	if ( wrap && begin.getY() == end.getY() ) {
		gotoxy( mBuffer->textline( begin.getY() ).length(), begin.getY(), false );
		lineDY = mainCursor->screenY();
	}
	gotoxy( begin.getX() ? begin.getX() - 1 : 0, begin.getY(), false );
}

void YZView::applyDeleteLine( const YZCursor& begin, const YZCursor& end, bool applyCursor ) {
	unsigned int dY = mainCursor->screenY();
	if ( begin.getY() == end.getY() ) {
		if ( wrap ) { // if current line take more/less lines, repaint all bottom
			gotoxy( mBuffer->textline( begin.getY() ).length(), begin.getY(), false );
			if ( lineDY != mainCursor->screenY() )
				paintEvent( dCurrentLeft, dY, mColumnsVis, mLinesVis - ( dY - dCurrentTop ) );
			else
				paintEvent( dCurrentLeft, dY, mColumnsVis, 1 );
		} else
			paintEvent( dCurrentLeft, dY, mColumnsVis, 1 );

	} else
		paintEvent( dCurrentLeft, dY, mColumnsVis, mLinesVis - ( dY - dCurrentTop ) );

	if ( applyCursor )
		gotoxy( begin.getX(), begin.getY() );
	else
		gotoxy( origPos->getX(), origPos->getY() );
	UPDATE_STICKY_COL;
}



QString YZView::deleteCharacter( const QString& , YZCommandArgs args ) {
	mBuffer->action()->deleteChar( this, mainCursor->buffer(), args.count );
	purgeInputBuffer();
	return QString::null;
}

QString YZView::changeLine ( const QString& inputsBuff, YZCommandArgs args ) {
	if ( args.command == "cc" ) args.command = "dd";
	else if ( args.command == "C" ) args.command = "D";
	else args.command = args.command.replace( 0, 1, "d" );
	deleteLine( inputsBuff, args );
	gotoInsertMode( );
	return QString::null;
}

QString YZView::deleteLine ( const QString& /*inputsBuff*/, YZCommandArgs args ) {
	int nb_lines=args.count;
	QChar reg=args.registr;

	QStringList buff; //to copy old lines into the register "
	unsigned int mY = mainCursor->bufferY();
//	unsigned int mX = mCursor->getX();
	if ( args.command == "dd" ) { //delete whole lines
		buff << QString::null;
		for ( int i = 0; i < nb_lines && ( mY + i ) < ( unsigned int )mBuffer->lineCount(); i++ )
			buff << mBuffer->textline( mY + i );
		buff << QString::null;
		mBuffer->action()->deleteLine( this, mainCursor->buffer(), nb_lines );
		YZSession::mRegisters.setRegister( reg, buff );
		purgeInputBuffer();
		return QString::null;
	} else if ( args.command == "D" ) { //delete to end of lines
		//just fake the real command
		args.command = "d$";
		args.motion = "$";
	} 
	if ( ( mMode == YZ_VIEW_MODE_VISUAL || mMode == YZ_VIEW_MODE_VISUAL_LINE ) && args.command.startsWith( "d" ) ) {
		YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
		selectionPool->clear( "VISUAL" );
		mBuffer->action()->deleteArea( this, cur_sel.from(), cur_sel.to( ), reg);
		gotoCommandMode();
	} else if (args.command.startsWith("d")) {
		if ( ! mSession->getMotionPool()->isValid( args.motion ) ) return QString::null; //keep going waiting for new inputs
		//ok we have a motion , so delete till the end of the motion :)
		YZCursor from(mainCursor->buffer()), to(this);
		bool success = mSession->getMotionPool()->applyMotion(args.motion, this, from, to);
		bool goBack = to < from;
		if ( !success ) {
			purgeInputBuffer();
			return QString::null;
		}
		//delete to the cursor position now :)
		yzDebug() << "Start of motion is : " << from << endl;
		yzDebug() << "End of motion is : " << to << endl;

		if ( goBack ) {
			YZCursor tmp( to );
			to.setCursor( from );
			from.setCursor( tmp );
		}

		mBuffer->action()->deleteArea( this, from, to, reg );
	}
	//reset the input buffer
	purgeInputBuffer();

	return QString::null;
}


QString YZView::openNewLineBefore ( const QString&, YZCommandArgs ) {
	mBuffer->action()->insertNewLine( this, 0, mainCursor->bufferY() );
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();
	return QString::null;
}

QString YZView::openNewLineAfter ( const QString&, YZCommandArgs ) {
	mBuffer->action()->insertNewLine( this, 0, mainCursor->bufferY() + 1 );
	purgeInputBuffer();
	gotoInsertMode();
	return QString::null;
}

QString YZView::append ( const QString&, YZCommandArgs ) {
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();
	gotoxy(mainCursor->bufferX()+1, mainCursor->bufferY() );
	UPDATE_STICKY_COL;

	return QString::null;
}

QString YZView::appendAtEOL ( const QString&, YZCommandArgs ) {
	//reset the input buffer
	purgeInputBuffer();
	moveToEndOfLine();
	append();

	return QString::null;
}

void YZView::switchModes(int mode) {
	if (mode != mMode) {
		mPrevMode = mMode;
		mMode = static_cast<modeType>(mode);
		modeChanged();
	}
}

QString YZView::gotoPreviousMode() {
	yzDebug() << "previous mode is  : " << getPreviousMode() << endl;
	if (getPreviousMode()==YZ_VIEW_MODE_OPEN)
	{
		yzDebug() << "switching to Open Mode" <<endl;
		return gotoOpenMode();
	}
	else if (getCurrentMode()==YZ_VIEW_MODE_OPEN)
	{
		yzDebug() << "Not switching modes" << endl;
		return QString::null;
	}
	yzDebug() << "switching to Command Mode" <<endl;
	return gotoCommandMode();
}

QString YZView::gotoCommandMode( ) {
	mBuffer->undoBuffer()->commitUndoItem(mainCursor->bufferX(), mainCursor->bufferY());
	switchModes(YZ_VIEW_MODE_COMMAND);
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoExMode(const QString&, YZCommandArgs ) {
	switchModes(YZ_VIEW_MODE_EX);
	mSession->setFocusCommandLine();
	purgeInputBuffer();
	setCommandLineText( "" );
	return QString::null;
}

QString YZView::gotoOpenMode(const QString &, YZCommandArgs ) {
	switchModes(YZ_VIEW_MODE_OPEN);
	setVisibleArea(80, 1);
	purgeInputBuffer();
	setCommandLineText("");
	yzDebug() << "successfully set open mode" <<endl;
	return QString::null;
}

QString YZView::gotoInsertMode(const QString&, YZCommandArgs ) {
	mBuffer->undoBuffer()->commitUndoItem(mainCursor->bufferX(), mainCursor->bufferY());
	switchModes(YZ_VIEW_MODE_INSERT);
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoReplaceMode(const QString&, YZCommandArgs ) {
	mBuffer->undoBuffer()->commitUndoItem(mainCursor->bufferX(), mainCursor->bufferY());
	switchModes(YZ_VIEW_MODE_REPLACE);
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoSearchMode( const QString& inputsBuff, YZCommandArgs /*args */) {
	reverseSearch = (inputsBuff[ 0 ] == '?' );
	switchModes(YZ_VIEW_MODE_SEARCH);
	purgeInputBuffer();
	setCommandLineText( "" );
	return QString::null;
}

QString YZView::gotoVisualMode(const QString& inputsBuff, YZCommandArgs ) {
	//store the from position
	if ( inputsBuff.startsWith( "V" ) )
		switchModes( YZ_VIEW_MODE_VISUAL_LINE );
	else
		switchModes( YZ_VIEW_MODE_VISUAL );
	*mVisualCursor = *mainCursor->buffer();
	*dVisualCursor = *mainCursor->screen();

	YZCursor bEnd( *mainCursor->buffer() );
	YZCursor dEnd( *mainCursor->screen() );

	if ( mMode == YZ_VIEW_MODE_VISUAL_LINE ) {
		mVisualCursor->setX( 0 );
		dVisualCursor->setX( 0 );
		bEnd.setX( mBuffer->textline( bEnd.getY() ).length() );
	}

	selectionPool->clear( "VISUAL" );
	selectionPool->addSelection( "VISUAL", *mVisualCursor, bEnd, *dVisualCursor, dEnd );
	paintEvent( dCurrentLeft, dVisualCursor->getY(), mColumnsVis, 1 );
	yzDebug("Visual mode") << "Starting at " << *mVisualCursor << endl;
	purgeInputBuffer();
	return QString::null;
}

void YZView::leaveVisualMode( ) {
	YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
	selectionPool->clear( "VISUAL" );
	paintEvent( dCurrentLeft, cur_sel.drawFrom().getY() > 0 ? cur_sel.drawFrom().getY() - 1 : 0, mColumnsVis, cur_sel.drawTo().getY() - cur_sel.drawFrom().getY() + 3 );
	gotoCommandMode();
}


QString YZView::addMark( const QString& inputsBuff, YZCommandArgs ) {
	mBuffer->marks()->add( inputsBuff[ 1 ], mainCursor->buffer(), mainCursor->screen() );
	purgeInputBuffer();
	return QString::null;
}

QString YZView::copy( const QString& , YZCommandArgs args) {
	//default register to use
	int nb_lines = args.count;
	
	QStringList list;
	if ( args.command == "yy" ) {
		list << QString::null; //just a marker
		for (int i = 0 ; i < nb_lines; i++ )
			list << mBuffer->textline(mainCursor->bufferY()+i);
		list << QString::null;
	} else if ( args.command == "Y" ) {
		list << QString::null; //just a marker
		for (int i = 0 ; i < nb_lines; i++ )
			list << mBuffer->textline(mainCursor->bufferY()+i);
		list << QString::null;
	} else if ( args.command == "y$" ) {
		QString lin = mBuffer->textline( mainCursor->bufferY() );
		list << lin.mid(mainCursor->bufferX());
	} else if ( ( mMode == YZ_VIEW_MODE_VISUAL || mMode == YZ_VIEW_MODE_VISUAL_LINE ) && args.command.startsWith( "y" ) ) {

		YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];

		list = mBuffer->getText( cur_sel.from(), cur_sel.to() );
		if ( mMode == YZ_VIEW_MODE_VISUAL_LINE ) {
			list.insert( list.begin(), QString::null );
			list.append( QString::null );
		}

		leaveVisualMode();

	} else if ( args.command.startsWith( "y" ) ) {
		if ( ! mSession->getMotionPool()->isValid( args.motion ) ) return QString::null; //keep going waiting for new inputs
		//ok we have a motion , so delete till the end of the motion :)
		YZCursor from(mainCursor->buffer()),to(this);
		bool success = mSession->getMotionPool()->applyMotion(args.motion,this,from,to);
		bool goBack = to < from;
		if ( !success ) {
			purgeInputBuffer();
			return QString::null;
		}
		//copy to the cursor position now :)
		yzDebug() << "Start of motion is : " << from << endl;
		yzDebug() << "End of motion is : " << to << endl;

		if ( goBack ) {
			YZCursor tmp( to );
			to.setCursor( from );
			from.setCursor( tmp );
		}

		bool entire = args.motion.startsWith( "'" ); // copy entire lines
		if ( entire ) {
			from.setX( 0 );
			to.setX( mBuffer->textline( to.getY() ).length() );
		}
		list = mBuffer->getText(from, to);
		if ( entire ) {
			list.insert( list.begin(), QString::null );
			list.append( QString::null );
		}
	}
	
	YZSession::mRegisters.setRegister( args.registr, list );

	purgeInputBuffer();
	return QString::null;
}

QString YZView::paste( const QString& , YZCommandArgs args ) {
	QStringList list = YZSession::mRegisters.getRegister( args.registr );

	YZCursor pos( mainCursor->buffer() );

	uint i = 0;
	bool copyWholeLinesOnly = list[ 0 ].isNull();
	QString copy = mBuffer->textline( pos.getY() );
	if ( args.command == "p" || ! copyWholeLinesOnly ) { //paste after current char
		if ( !list[ 0 ].isNull() ) {
			unsigned int start;
			if( args.command = "p" )
				start = copy.length() > 0 ? pos.getX() + 1 : 0;
			else
				start = pos.getX();
			yzDebug() << "First line not NULL !" << endl;
			copy = copy.mid( start );
			mBuffer->action()->deleteChar( this, start, pos.getY(), copy.length() );
			mBuffer->action()->insertChar( this, start, pos.getY(), list[ 0 ] );
		}
		i++;
		while ( i < list.size() - 1 ) {
			mBuffer->action()->insertLine( this, pos.getY() + i, list[ i ] );
			i++;
		}
		if ( ! list[ i ].isNull() ) {
			mBuffer->action()->insertChar( this, 0, pos.getY() + i, list[ i ] + copy );
			gotoxy( list[ i ].length() - 1, pos.getY() + i );
		}

	} else if ( args.command == "P" ) { //paste whole lines before current char
		for( i = 1; i < list.size() - 1; i++ ) 
			mBuffer->action()->insertLine( this, pos.getY() + i - 1, list[ i ] );

		gotoxy( pos.getX(), pos.getY() );
	}
	purgeInputBuffer();
	return QString::null;
}

bool YZView::doSearch( const QString& search ) {
	//build the regexp
	QRegExp ex( search );
	int currentMatchLine = mainCursor->bufferY(); //start from current line
	//if you understand this line you are semi-god :)
	//if you don't understand, remove the IFs and see what it does when you have the cursor on the last/first column and start a search/reverse search
	unsigned int mMaxX = mBuffer->textline( mainCursor->bufferY() ).length() - 1;
	int currentMatchColumn = reverseSearch ? ( mainCursor->bufferX() ? mainCursor->bufferX() : 1 ) - 1 : \
		( mainCursor->bufferX() < mMaxX ) ? mainCursor->bufferX() + 1 : mainCursor->bufferX(); //start from current column +/- 1

	//get current line
	QString l;

	selectionPool->clear( "SEARCH" );

	for ( unsigned int i = currentMatchLine; i < mBuffer->lineCount(); reverseSearch ? i-- : i++ ) {
		l = mBuffer->textline( i );
		yzDebug() << "Searching " << search << " in line : " << l << endl;
		int idx;
		if ( reverseSearch ) {
			idx = ex.searchRev( l, currentMatchColumn );
		} else
			idx = ex.search( l, currentMatchColumn );

		if ( idx >= 0 ) {
			//i really found it ? or is it a previous "found" ?
			if ( mainCursor->bufferX() == ( unsigned int ) idx ) { //ok we did not move guy (col 0 or last col maybe ...)
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
			selectionPool->addSelection( "SEARCH", currentMatchColumn, currentMatchLine, currentMatchColumn + ex.matchedLength() - 1, currentMatchLine );
			gotoxy( currentMatchColumn, currentMatchLine );
			UPDATE_STICKY_COL;
			refreshScreen( );
			return true;
		} else {
			yzDebug() << "No match on this line" << endl;
			if ( reverseSearch )
				currentMatchColumn=-1;
			else
				currentMatchColumn=0; //reset the column (only valid for the first line we check)
		}
	}
	refreshScreen( );
	return false;
}

QString YZView::searchAgain( const QString& /*inputsBuff*/, YZCommandArgs args ) {
	if ( mCurrentSearchItem == 0 ) return QString::null; //no previous search ;)

	for ( uint i = 0; i < args.count; i++ )  //search count times
	 	doSearch( mSearchHistory[mCurrentSearchItem-1] );
	purgeInputBuffer();
	return QString::null;
}

/*
 * Drawing engine
 */

void YZView::setFixedFont( bool fixed ) {
	isFontFixed = fixed;
	spaceWidth = GET_CHAR_WIDTH( ' ' );
}

bool YZView::isColumnVisible( unsigned int column, unsigned int  ) {
	return ! (column < dCurrentLeft || column >= (dCurrentLeft + mColumnsVis));
}


/* update sCurLine informations */
void YZView::updateCurLine( ) {
	sCurLineLength = sCurLine.length();
	if ( wrap && ! drawMode ) {
		unsigned int nbTabs = sCurLine.contains( '\t' );
		if ( isFontFixed ) rMinCurLineLength = sCurLineLength;
		else rMinCurLineLength = GET_STRING_WIDTH( QString( sCurLine ).remove( '\t' ) ) + nbTabs * spaceWidth;
		rCurLineLength = rMinCurLineLength + nbTabs * ( tablength - spaceWidth );
	}
}

void YZView::initDraw( ) {
	initDraw( mCurrentLeft, mCurrentTop, dCurrentLeft, dCurrentTop );
}

/**
 * initDraw is called before each cursor/draw manipulation.
 */
void YZView::initDraw( unsigned int sLeft, unsigned int sTop, unsigned int rLeft, unsigned int rTop, bool draw ) {
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

	workCursor->lineHeight = 0;

	workCursor->spaceFill = 0;

	adjust = false;

	tabstop = getLocalIntOption("tabstop");
	wrap = getLocalBoolOption( "wrap" );

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
		workCursor->setBufferY( workCursor->bufferY() - workCursor->sLineIncrement );
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
		YZLine *yl = mBuffer->yzline( workCursor->bufferY() );
		if ( ! workCursor->wrapNextLine ) {
			sCurLine = yl->data();
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
		workCursor->setBufferY( workCursor->bufferY() + workCursor->bLineIncrement );
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
	}
	workCursor->setScreenY( workCursor->screenY() + workCursor->sLineIncrement );
	workCursor->sLineIncrement = 1;

	if ( workCursor->bufferY() < mBuffer->lineCount() ) {
		YZLine *yl = mBuffer->yzline( workCursor->bufferY() );
		if ( ! workCursor->wrapNextLine ) {
			sCurLine = yl->data();
			updateCurLine( );
		}
		if ( rCurrentLeft > 0 && ! workCursor->wrapNextLine ) {
			workCursor->setBufferX( 0 );
			workCursor->setScreenX( 0 );
			adjust = true;
			gotodx( rCurrentLeft );
			adjust = false;
			if ( drawMode ) {
				if ( mCurrentLeft > 0 )
					workCursor->spaceFill = ( tablength - mCurrentLeft % tablength ) % tablength;
				if ( workCursor->screenX() > rCurrentLeft ) {
					workCursor->setBufferX( workCursor->bufferX() - 1 );
					workCursor->setScreenX( rCurrentLeft );
				}
			}
		}
		if ( drawMode && ( workCursor->screenY() - rCurrentTop ) < mLinesVis ) {
			rHLa = NULL;
			if ( yl->length() != 0 )
				rHLa = yl->attributes();
			rHLnoAttribs = !rHLa;
			rHLa = rHLa + workCursor->bufferX() - 1;
			rHLAttributes = 0L;
			YzisHighlighting * highlight = mBuffer->highlight();
			if ( highlight )
				rHLAttributes = highlight->attributes( 0 )->data( );
			rHLAttributesLen = rHLAttributes ? highlight->attributes( 0 )->size() : 0;
			return true;
		}
	} else {
		sCurLine = "";
		sCurLineLength = sCurLine.length();
	}
	workCursor->wrapNextLine = false;
	return false;
}

#define WATCHLINE 1
#define DEBUG_LINE if ( drawMode && workCursor->bufferY() == WATCHLINE ) yzDebug()

bool YZView::drawPrevCol( ) {
	workCursor->wrapNextLine = false;
	unsigned int shift = ( ! drawMode && ( YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT==mMode && sCurLineLength > 0 ) ) ? 1 : 0;
	if ( workCursor->bufferX() >= workCursor->bColIncrement ) {
		unsigned int curx = workCursor->bufferX() - 1;
		workCursor->setBufferX( curx );
		lastChar = sCurLine[ curx ];
		if ( lastChar != tabChar ) {
			workCursor->sColIncrement = GET_CHAR_WIDTH( lastChar );
			if ( workCursor->screenX() >= workCursor->sColIncrement )
				workCursor->setScreenX( workCursor->screenX() - workCursor->sColIncrement );
			else
				workCursor->wrapNextLine = ( wrap && rCurLineLength > mColumnsVis - shift && workCursor->screenX() == 0 && workCursor->bufferX() > 0 );
			workCursor->bLineIncrement = workCursor->wrapNextLine ? 0 : 1;
		} else {
			/* go back to begin of line */
//			DEBUG_LINE << "rLineHeight=" << rLineHeight << " at " << *rCursor << endl;
			initDraw( 0, workCursor->bufferY(), 0, workCursor->screenY() - workCursor->lineHeight + 1, drawMode );
			return false;
		}
	}
	return ! workCursor->wrapNextLine;
}

bool YZView::drawNextCol( ) {
//	if ( drawMode ) yzDebug() << "drawNextCol at " << *sCursor << "," << *rCursor << endl;
	bool ret = false;
	unsigned int curx = workCursor->bufferX();
	workCursor->wrapNextLine = false;

//	DEBUG_LINE << "init drawNextCol b=" << *workCursor->buffer() << ";s=" << *workCursor->screen() << endl;

	// keep value
	bool lastCharWasTab = workCursor->lastCharWasTab;

	unsigned int nextLength = 0;
	if ( ! drawMode ) nextLength = spaceWidth;

	workCursor->sColIncrement = spaceWidth;

	workCursor->lastCharWasTab = false;
	if ( curx < sCurLineLength ) {
		unsigned int lenToTest;
		lastChar = sCurLine[ curx ];
		if ( drawMode ) charSelected = selectionPool->isSelected( workCursor->buffer() );
		if ( lastChar != tabChar ) {
			workCursor->sColIncrement = GET_CHAR_WIDTH( lastChar );
			lenToTest = workCursor->sColIncrement;
		} else {
			workCursor->lastCharWasTab = true;
			lastChar = ' ';
			if ( workCursor->screenX( ) == mCurrentLeft ) 
				workCursor->sColIncrement = ( workCursor->spaceFill ? workCursor->spaceFill : tablength );
			else {
				// calculate tab position
				unsigned int mySpaceFill = ( mCurrentLeft == 0 ? workCursor->spaceFill : 0 );	// do not care about rSpaceFill if we arent't in wrapped mode
				if ( workCursor->screenX() >= mySpaceFill ) 
					workCursor->sColIncrement = ( ( workCursor->screenX() - mySpaceFill ) / tablength  + 1 ) * tablength + mySpaceFill - workCursor->screenX();
				else
					workCursor->sColIncrement = mySpaceFill + spaceWidth - workCursor->screenX();
			}
			if ( drawMode ) lenToTest = spaceWidth;
			else lenToTest = workCursor->sColIncrement;
		}
		if ( ! drawMode && ! isFontFixed && workCursor->bufferX() + workCursor->bColIncrement < sCurLineLength )
			nextLength = GET_CHAR_WIDTH( sCurLine[ workCursor->bufferX() + workCursor->bColIncrement ] );

		// will our new char appear in the area ?
		ret = adjust || workCursor->screenX() + lenToTest - mCurrentLeft <= mColumnsVis - nextLength;

		if ( ret || ! drawMode ) {
			// moving cursors
			workCursor->setScreenX( workCursor->screenX() + workCursor->sColIncrement );
			workCursor->setBufferX( workCursor->bufferX() + workCursor->bColIncrement );
			// update HL
			if ( drawMode ) rHLa += workCursor->sColIncrement;
		}
	}

	// can we go after the end of line buffer ?
	unsigned int shift = ( ! drawMode && ( YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT == mMode && sCurLineLength > 0 ) ) ? 1 : 0;

	// drawCursor is after end of area ?
	workCursor->wrapNextLine = ( wrap && workCursor->screenX() + ( ret || ! drawMode ? 0 : workCursor->sColIncrement ) > mColumnsVis - nextLength && curx < sCurLineLength + shift );

	// only remember of case where wrapNextLine is true ( => we will wrap a tab next drawNextCol )
	if ( workCursor->lastCharWasTab ) workCursor->lastCharWasTab = workCursor->wrapNextLine;

	// wrapNextLine is true, we are out of area ( ret is false ), last char was a tab => we are wrapping a tab
	if ( workCursor->wrapNextLine ) {
		if ( drawMode )	workCursor->wrapTab = ! ret && lastCharWasTab;
		else workCursor->wrapTab = workCursor->lastCharWasTab && workCursor->screenX() > mColumnsVis;
	}

	// do not increment line buffer if we are wrapping a line
	workCursor->bLineIncrement = workCursor->wrapNextLine ? 0 : 1;

//	if ( drawMode && mCurrentLeft > 0 )
//		yzDebug() << "done drawNextCol s=" << *sCursor << ";r=" << *rCursor << ";ret=" << ret << ";wrapNextLine=" 
//			<< wrapNextLine << ";rLastCharWasTab=" << rLastCharWasTab << ";wrapTab=" << wrapTab << endl;

	return ret;
}

const QChar& YZView::drawChar( ) {
	return lastChar;
}
unsigned int YZView::drawLength( ) {
	return workCursor->sColIncrement;
}
unsigned int YZView::drawHeight ( ) {
	return workCursor->sLineIncrement;
}
unsigned int YZView::lineHeight ( ) {
	return workCursor->bLineIncrement;
}
bool YZView::drawSelected( ) {
	return charSelected;
}

const QColor& YZView::drawColor ( unsigned int col, unsigned int line ) {
	YZLine *yl = mBuffer->yzline( line );
	YzisHighlighting * highlight = mBuffer->highlight();
	const uchar* hl=NULL;

	if ( yl->length() != 0 && highlight ) {
		hl = yl->attributes(); //attributes of this line
		hl += col; // -1 ? //move pointer to the correct column
		uint len = hl ? highlight->attributes( 0 )->size() : 0 ; //length of attributes
		YzisAttribute *list = highlight->attributes( 0 )->data( ); //attributes defined by the syntax highlighting document
		YzisAttribute *at = ( ( *hl ) >= len ) ? &list[ 0 ] : &list[*hl]; //attributes pointed by line's attribute for current column
		if ( at ) return at->textColor(); //textcolor :)
	}

	return fake;
}

const QColor& YZView::drawColor ( ) {
	YzisAttribute * curAt = ( !rHLnoAttribs && (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt ) return curAt->textColor();
	return fake;
}

unsigned int YZView::drawLineNumber( ) {
	return workCursor->bufferY( ) + 1;
}

unsigned int YZView::drawTotalHeight() {
	unsigned int totalHeight = 0;

	initDraw( 0, 0, 0, 0 );
	drawMode = false;
	while( workCursor->bufferY() < mBuffer->lineCount() ) {
		drawNextLine();
		totalHeight += drawHeight();
	}

	return totalHeight;
}
	

void YZView::substitute(const QString& range, const QString& search, const QString& replace, const QString& option) {
	yzDebug() << "substitute : " << range << ":" << search << ":" << replace << ":" << option << endl;
	//TODO : better range support, better options support
	unsigned int startLine = mainCursor->bufferY();
	unsigned int endLine = mainCursor->bufferY();
	bool needsUpdate=false;
	//whole file
	if ( range == "%" ) {
		startLine = 0;
		endLine = mBuffer->lineCount()-1;
	} else if ( range.contains( "," ) ) {
		QStringList list = QStringList::split( ",", range, TRUE);
		yzDebug() << "sub tmp: " << list.size() <<endl;
		if (list.size()==2) {	// don't use this if the user made a mistake
			yzDebug() << "substitute tmp : " << list[0] << ":" << list[1]
				<< endl;
			startLine=list[0].toUInt()-1;
			endLine=list[1].toUInt()-1;
			if (endLine>=mBuffer->lineCount())
				endLine=mBuffer->lineCount()-1;
		}
		
	} else if ( range == "." ) {
		//nothing
	}
		
	for ( unsigned int i = startLine; i <= endLine; i++ ) {
		if ( mBuffer->substitute(search, replace, option.contains( "g" ), i) )
			needsUpdate = true;
	}
	mBuffer->updateAllViews();
}

void YZView::joinLine( unsigned int line ) {
	if ( line >= mBuffer->lineCount() - 1 ) return;
	gotoxy( mBuffer->textline( line ).length() - 1, line );
	UPDATE_STICKY_COL;
	mBuffer->mergeNextLine( line );
	paintEvent( dCurrentLeft, mainCursor->screenY(), mColumnsVis, mLinesVis - ( mainCursor->screenY() - dCurrentTop ) );
}

QString YZView::joinLine ( const QString& , YZCommandArgs ) {
	//reset the input buffer
	purgeInputBuffer();
	joinLine( mainCursor->bufferY() );
	return QString::null;
}

void YZView::printToFile( const QString& path ) {
	YZPrinter printer( this );
	printer.printToFile( path );
	printer.run( );
}

QString YZView::undo( const QString& , YZCommandArgs ) {
	/* XXX repeat if necessary */
	mBuffer->undoBuffer()->undo( this );
	//reset the input buffer of the originating view
	purgeInputBuffer();
	return QString::null;
}

QString YZView::redo( const QString& , YZCommandArgs ) {
	/* XXX repeat if necessary */
	mBuffer->undoBuffer()->redo( this );
	//reset the input buffer of the originating view
	purgeInputBuffer();
	return QString::null;
}

QString YZView::match( const QString&, YZCommandArgs ) {
	bool found = false;
	YZCursor pos = mBuffer->action()->match( this, *mainCursor->buffer(), &found );
	yzDebug() << "Result " << pos << endl;

	if ( found )
		gotoxy( pos.getX(), pos.getY() );

	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoMark( const QString& inputsBuff, YZCommandArgs ) {
	yzDebug() << "gotoMark " << inputsBuff.mid( 1,1 ) << endl;
	bool found = false;
	YZCursorPos pos = mBuffer->marks()->get(inputsBuff.mid( 1, 1 ), &found);
	if ( found ) {
		gotoxy(pos.bPos->getX(), pos.bPos->getY());
	}
	purgeInputBuffer();
	return QString::null;

}

QString YZView::refreshScreenInternal(const QString& , YZCommandArgs ) {
	purgeInputBuffer();
	refreshScreen();
	return QString::null;
}

int YZView::getLocalIntOption( const QString& option ) {
	if ( YZSession::mOptions.hasOption( mBuffer->fileName()+"-view-"+ QString::number(myId) +"\\"+option ) ) //find the local one ?
		return YZSession::mOptions.readIntEntry( mBuffer->fileName()+"-view-"+ QString::number(myId) +"\\"+option, 0 );
	else
		return YZSession::mOptions.readIntEntry( "Global\\" + option, 0 ); // else give the global default if any
}

void YZView::setLocalIntOption( const QString& key, int option ) {
	YZSession::mOptions.setGroup( mBuffer->fileName()+"-view-"+ QString::number(myId) );
	YZSession::mOptions.setIntOption( key, option );
}

bool YZView::getLocalBoolOption( const QString& option ) {
	if ( YZSession::mOptions.hasOption( mBuffer->fileName()+"-view-"+ QString::number(myId) +"\\"+option ) )
		return YZSession::mOptions.readBoolEntry( mBuffer->fileName()+"-view-"+ QString::number(myId) +"\\"+option, false );
	else
		return YZSession::mOptions.readBoolEntry( "Global\\" + option, false );
}

void YZView::setLocalBoolOption( const QString& key, bool option ) {
	YZSession::mOptions.setGroup( mBuffer->fileName()+"-view-"+ QString::number(myId) );
	YZSession::mOptions.setBoolOption( key, option );
}

QString YZView::getLocalStringOption( const QString& option ) {
	if ( YZSession::mOptions.hasOption( mBuffer->fileName()+"-view-"+ QString::number(myId) +"\\"+option ) )
		return YZSession::mOptions.readQStringEntry( mBuffer->fileName()+"-view-"+ QString::number(myId) +"\\"+option, QString("") );
	else
		return YZSession::mOptions.readQStringEntry( "Global\\" + option, QString("") );
}

void YZView::setLocalQStringOption( const QString& key, const QString& option ) {
	YZSession::mOptions.setGroup( mBuffer->fileName()+"-view-"+ QString::number(myId) );
	YZSession::mOptions.setQStringOption( key, option );
}

QStringList YZView::getLocalStringListOption( const QString& option ) {
	if ( YZSession::mOptions.hasOption( mBuffer->fileName()+"-view-"+ QString::number(myId) +"\\"+option ) )
		return YZSession::mOptions.readQStringListEntry( mBuffer->fileName()+"-view-"+ QString::number(myId) +"\\"+option, QStringList::split(";","") );
	else
		return YZSession::mOptions.readQStringListEntry( "Global\\" + option, QStringList::split(";","") );
}

void YZView::setLocalQStringListOption( const QString& key, const QStringList& option ) {
	YZSession::mOptions.setGroup( mBuffer->fileName()+"-view-"+ QString::number(myId) );
	YZSession::mOptions.setQStringListOption( key, option );
}

QColor YZView::getLocalColorOption( const QString& option ) {
	if ( YZSession::mOptions.hasOption( mBuffer->fileName()+"-view-"+ QString::number(myId) +"\\"+option ) )
		return YZSession::mOptions.readQColorEntry( mBuffer->fileName()+"-view-"+ QString::number(myId) +"\\"+option, QColor("white") );
	else
		return YZSession::mOptions.readQColorEntry( "Global\\" + option, QColor("white") );
}

void YZView::setLocalQColorOption( const QString& key, const QColor& option ) {
	YZSession::mOptions.setGroup( mBuffer->fileName()+"-view-"+ QString::number(myId) );
	YZSession::mOptions.setQColorOption( key, option );
}

