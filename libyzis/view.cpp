/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <marchand@kde.org>
 *  Thomas Capricelli <orzel@freehackers.org>.
 *  Loic Pauleve <panard@inzenet.org>
 *  Pascal "Poizon" Maillard <poizon@gmx.at>
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
#include "viewcursor.h"
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

#define GET_STRING_WIDTH( s ) ( isFontFixed ? s.length() : stringWidth( s ) )
#define GET_CHAR_WIDTH( c ) ( isFontFixed ? 1 : charWidth( c ) )

static const QChar tabChar( '\t' );
static QColor fake;


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

	rHLnoAttribs = false;
	rHLAttributesLen = 0;
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

void YZView::sendKey( const QString& _key, const QString& modifiers) {
	yzDebug() << "sendKey : " << _key << " " << modifiers << endl;
	//TODO swapfile
	//mBuffer->getSwapFile()->addToSwap( c, modifiers );
	if ( mBuffer->introShown() ) {
		mBuffer->clearIntro();
		gotoxy( 0,0 );
		return;
	}

	QString lin,key=_key;
	if ( _key == "<SHIFT>" || _key == "<CTRL>" || _key == "<ALT" ) return; //we are not supposed to received modifiers in key

	if ( modifiers.contains ("<SHIFT>")) //usefull ?
		key = key.upper();
	
	bool test = false;

	bool cindent = getLocalBoolOption( "cindent" );

	switch(mMode) {
		case YZ_VIEW_MODE_INSERT:
			if ( key == "<HOME>" ) {
				moveToStartOfLine( );
				return;
			} else if ( key == "<END>" ) {
				moveToEndOfLine( );
				return;
			} else if ( key == "<INS>" ) {
				gotoReplaceMode( );
				return;
			} else if ( key == "<ESC>" ) {
				if ( mainCursor->bufferX() > 0) moveLeft();
				gotoPreviousMode();
				return;
			} else if ( key == "<ENTER>" ) {
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
					updateStickyCol( mainCursor );
				}
				return;
			} else if ( key == "<DOWN>" ) {
				moveDown( );
				return;
			} else if ( key == "<LEFT>" ) {
				moveLeft();
				return;
			} else if ( key == "<RIGHT>" ) {
				moveRight();
				return;
			} else if ( key == "<UP>" ) {
				moveUp( );
				return;
			} else if ( key == "<BS>" ) {
				if (mainCursor->bufferX() == 0 && mainCursor->bufferY() > 0 && getLocalStringOption( "backspace" ).contains( "eol" ) ) {
					joinLine( mainCursor->bufferY() - 1 );
					if ( mainCursor->bufferX() ) moveRight();
				} else if ( mainCursor->bufferX() > 0 )
					mBuffer->action()->deleteChar( this, mainCursor->bufferX() - 1, mainCursor->bufferY(), 1 );
				return;
			} else if ( key == "<DEL>" ) {
				mBuffer->action()->deleteChar( this, mainCursor->buffer(), 1 );
				return;
			} else if ( key == "<PDOWN>" ) {
				gotoStickyCol( mainCursor, mainCursor->bufferY() + mLinesVis );
				purgeInputBuffer();
				return;
			} else if ( key == "<PUP>" ) {
				gotoStickyCol( mainCursor, mainCursor->bufferY() > mLinesVis ? mainCursor->bufferY() - mLinesVis : 0 );
				purgeInputBuffer();
				return;
			} else {
				if ( key == "<TAB>" ) {
					key="\t";
				}
				mBuffer->action()->insertChar( this, mainCursor->buffer(), key );
				if ( cindent && key == "}" )
					reindent(mainCursor->bufferX()-1, mainCursor->bufferY());
				return;
			}
			break;

		case YZ_VIEW_MODE_REPLACE:
			if ( key == "<HOME>" ) {
				moveToStartOfLine( );
				return;
			} else if ( key == "<END>" ) {
				moveToEndOfLine( );
				return;
			} else if ( key == "<INS>" ) {
				gotoInsertMode( );
				return;
			} else if ( key == "<DEL>" ) {
				mBuffer->action()->deleteChar( this, mainCursor->buffer(), 1 );
				return;
			} else if ( key == "<ESC>" ) {
				if ( mainCursor->bufferX() == mBuffer->textline( mainCursor->bufferY() ).length() ) 
					moveToEndOfLine( );
				gotoPreviousMode();
				return;
			} else if ( key == "<RETURN>" ) {
				test = mainCursor->bufferX() == 0;
				mBuffer->action()->insertNewLine( this, mainCursor->buffer() );
				if ( test ) {
					gotoxy( 0, mainCursor->bufferY() + 1 );
					updateStickyCol( mainCursor );
				}
				return;
			} else if ( key == "<DOWN>" ) {
				moveDown( );
				return;
			} else if ( key == "<BS>" || key == "<LEFT>" ) {
				moveLeft();
				return;
			} else if ( key == "<RIGHT>" ) {
				moveRight();
				return;
			} else if ( key == "<UP>" ) {
				moveUp( );
				return;
			} else if ( key == "<TAB>" ) {
				mBuffer->action()->replaceChar( this, mainCursor->buffer(), "\t" );
				return;
			} else if ( key == "<PDOWN>" ) {
				gotoStickyCol( mainCursor, mainCursor->bufferY() + mLinesVis );
				purgeInputBuffer();
				return;
			} else if ( key == "<PUP>" ) {
				gotoStickyCol( mainCursor, mainCursor->bufferY() > mLinesVis ? mainCursor->bufferY() - mLinesVis : 0 );
				purgeInputBuffer();
				return;
			} else {
				mBuffer->action()->replaceChar( this, mainCursor->buffer(), key );
				return;
			}
			break;

		case YZ_VIEW_MODE_SEARCH:
			if ( key == "<ENTER>" ) {
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
			} else if ( key == "<DOWN>" ) {
				if(mSearchHistory[mCurrentSearchItem].isEmpty())
					return;

				mCurrentSearchItem++;
				setCommandLineText( mSearchHistory[mCurrentSearchItem] );
				return;
			} else if ( key == "<LEFT>" || key == "<RIGHT>" ) {
				return;
			} else if ( key == "<UP>" ) {
				if(mCurrentSearchItem == 0)
					return;

				mCurrentSearchItem--;
				setCommandLineText( mSearchHistory[mCurrentSearchItem] );
				return;
			} else if ( key == "<ESC>" ) {
				setCommandLineText( "" );
				mSession->setFocusMainWindow();
				gotoPreviousMode();
				return;
			} else if ( key == "<BS>" ) {
				QString back = getCommandLineText();
				setCommandLineText(back.remove(back.length() - 1, 1));
				return;
			} else {
				setCommandLineText( getCommandLineText() + key );
				return;
			}
			break;

		case YZ_VIEW_MODE_EX:
			if ( key == "<ENTER>" ) {
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
				return;
			} else if ( key == "<DOWN>" ) {
				if(mExHistory[mCurrentExItem].isEmpty())
					return;

				mCurrentExItem++;
				setCommandLineText( mExHistory[mCurrentExItem] );
				return;
			} else if ( key == "<LEFT>" || key == "<RIGHT>" ) {
				return;
			} else if ( key == "<UP>" ) {
				if(mCurrentExItem == 0)
					return;

				mCurrentExItem--;
				setCommandLineText( mExHistory[mCurrentExItem] );
				return;
			} else if ( key == "<ESC>" ) {
				setCommandLineText( "" );
				mSession->setFocusMainWindow();
				gotoPreviousMode();
				return;
			} else if ( key == "<TAB>" ) {
				//ignore for now
				return;
			} else if ( key == "<BS>" ) {
				QString back = getCommandLineText();
				if ( back.isEmpty() ) {
					mSession->setFocusMainWindow();
					gotoPreviousMode();
					return;
				}
				setCommandLineText(back.remove(back.length() - 1, 1));
				return;
			} else {
				setCommandLineText( getCommandLineText() + key );
				return;
			}
			break;

		case YZ_VIEW_MODE_VISUAL:
		case YZ_VIEW_MODE_VISUAL_LINE :
			if ( key == "<ESC>" ) {
				YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
				selectionPool->clear( "VISUAL" );
				paintEvent( dCurrentLeft, cur_sel.drawFrom().getY(), mColumnsVis, cur_sel.drawTo().getY() - cur_sel.drawFrom().getY() + 1 );
				purgeInputBuffer();
				gotoPreviousMode();
				return;
			}
			//dont break
		case YZ_VIEW_MODE_COMMAND:
			if ( key == "<ESC>" ) {
				purgeInputBuffer();
				return;
			} else if ( key == "<DEL>" ) {
				if ( mMode == YZ_VIEW_MODE_VISUAL || mMode == YZ_VIEW_MODE_VISUAL_LINE ) {
					YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
					mBuffer->action()->deleteArea( this, cur_sel.from(), cur_sel.to(), ( QValueList<QChar>() << QChar( '\"' ) ));
					selectionPool->clear( "VISUAL" );
					purgeInputBuffer();
					gotoCommandMode();
					return;
				} else
					mBuffer->action()->deleteChar( this, *mainCursor->buffer(), 1);
				return;
			} else if ( key == "<PDOWN>" ) {
				gotoStickyCol( mainCursor, mainCursor->bufferY() + mLinesVis );
				purgeInputBuffer();
				return;
			} else if ( key == "<PUP>" ) {
				gotoStickyCol( mainCursor, mainCursor->bufferY() > mLinesVis ? mainCursor->bufferY() - mLinesVis : 0 );
				purgeInputBuffer();
				return;
			}
			mPreviousChars+=key;
			if ( mSession ) {
				cmd_state state=mSession->getPool()->execCommand(this, mPreviousChars);
				yzDebug() << "Command " << mPreviousChars << " gave state: " << state << endl;
				switch(state) {
					case CMD_ERROR:
					case CMD_OK:
						purgeInputBuffer();
						break;
					default:
						break;
				}
			}
			break;

		case YZ_VIEW_MODE_OPEN:
			if ( key == ":" || key == "Q" ) {
				gotoExMode();
				break;
			} else {
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
		initGoto( mainCursor );
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
	if ( alignTop ) gotoStickyCol( mainCursor, mCurrentTop );
}

/* recalculate cursor position + refresh screen */
void YZView::reset( ) {
	initDraw( 0, 0, 0, 0 );
	drawMode = false;
	gotoy( mainCursor->bufferY() );
	gotox( mainCursor->bufferX() );
	applyGoto( mainCursor );
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

void YZView::initGoto( YZViewCursor* viewCursor ) {
	initDraw( viewCursor->bufferX(), viewCursor->bufferY(), viewCursor->screenX(), viewCursor->screenY(), false );
	*workCursor = *viewCursor;
}

void YZView::applyGoto( YZViewCursor* viewCursor, bool applyCursor ) {
	*viewCursor = *workCursor;

	if ( applyCursor && viewCursor != mainCursor ) { // do not apply if this isn't the mainCursor
		yzDebug() << "THIS IS NOT THE MAINCURSOR" << endl;
		applyCursor = false;
	}

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
			if (new_cur_sel.drawFrom() > cur_sel.drawFrom() ) new_cur_sel.setDrawFrom ( cur_sel.drawFrom() );
			if ( new_cur_sel.drawTo() < cur_sel.drawTo() ) new_cur_sel.setDrawTo( cur_sel.drawTo() );
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
	gotodxdy( mainCursor, nextx, nexty, applyCursor );
}
void YZView::gotodxdy( YZViewCursor* viewCursor, unsigned int nextx, unsigned int nexty, bool applyCursor ) {
	initGoto( viewCursor );
	gotody( nexty );
	gotodx( nextx );
	applyGoto( viewCursor, applyCursor );
}

/* goto xdraw, ybuffer */
void YZView::gotodxy( unsigned int nextx, unsigned int nexty, bool applyCursor ) {
	gotodxy( mainCursor, nextx, nexty, applyCursor );
}
void YZView::gotodxy( YZViewCursor* viewCursor, unsigned int nextx, unsigned int nexty, bool applyCursor ) {
	initGoto( viewCursor );
	gotoy( nexty );
	gotodx( nextx );
	applyGoto( viewCursor, applyCursor );
}

/* goto xdraw, ybuffer */
void YZView::gotoxdy( unsigned int nextx, unsigned int nexty, bool applyCursor ) {
	gotoxdy( mainCursor, nextx, nexty, applyCursor );
}
void YZView::gotoxdy( YZViewCursor* viewCursor, unsigned int nextx, unsigned int nexty, bool applyCursor ) {
	initGoto( viewCursor );
	gotody( nexty );
	gotox( nextx );
	applyGoto( viewCursor, applyCursor );
}

/* goto xbuffer, ybuffer */
void YZView::gotoxy(unsigned int nextx, unsigned int nexty, bool applyCursor ) {
	gotoxy( mainCursor, nextx, nexty, applyCursor );
}
void YZView::gotoxy( YZViewCursor* viewCursor, unsigned int nextx, unsigned int nexty, bool applyCursor ) {
	initGoto( viewCursor );
	gotoy( nexty );
	gotox( nextx );
	applyGoto( viewCursor, applyCursor );
}

QString YZView::moveDown( unsigned int nb_lines ) {
	return moveDown( mainCursor, nb_lines );
}
QString YZView::moveDown( YZViewCursor* viewCursor, unsigned int nb_lines ) {
	//execute the code
	unsigned int nextLine = QMIN( viewCursor->bufferY() + nb_lines, mBuffer->lineCount() - 1 );
	gotoStickyCol( viewCursor, nextLine );

	//return something
	return QString::null;
}

QString YZView::moveUp( unsigned int nb_lines ) {
	return moveUp( mainCursor, nb_lines );
}
QString YZView::moveUp( YZViewCursor* viewCursor, unsigned int nb_lines ) {
	//execute the code
	unsigned int nextLine = QMAX( viewCursor->bufferY() - nb_lines, 0 );
	gotoStickyCol( viewCursor, nextLine );

	//return something
	return QString::null;
}


QString YZView::moveLeft( int nb_cols, bool wrap ) {
	return moveLeft( mainCursor, nb_cols, wrap );
}
QString YZView::moveLeft( YZViewCursor* viewCursor, int nb_cols, bool wrap ) {
	int x=int(viewCursor->bufferX());
	unsigned int y=viewCursor->bufferY();
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
	gotoxy( viewCursor, (unsigned int)(x), y);

//	if ( viewCursor == mainCursor ) UPDATE_STICKY_COL;
	updateStickyCol( viewCursor );

	//return something
	return QString::null;
}

QString YZView::moveRight( int nb_cols, bool wrap ) {
	return moveRight( mainCursor, nb_cols, wrap );
}
QString YZView::moveRight( YZViewCursor* viewCursor, int nb_cols, bool wrap ) {
	unsigned int x=viewCursor->bufferX();
	unsigned int y=viewCursor->bufferY();
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
			x=myBuffer()->textline(y).length()-1;
	}
	gotoxy( viewCursor, (unsigned int)(x), y);

//	if ( viewCursor == mainCursor ) UPDATE_STICKY_COL;
	updateStickyCol( viewCursor );

	//return something
	return QString::null;
}

QString YZView::moveToFirstNonBlankOfLine( ) {
	return moveToFirstNonBlankOfLine( mainCursor );
}
QString YZView::moveToFirstNonBlankOfLine( YZViewCursor* viewCursor ) {
	//execute the code
	gotoxy( viewCursor, mBuffer->firstNonBlankChar(viewCursor->bufferY()) , viewCursor->bufferY());
//	if ( viewCursor == mainCursor ) UPDATE_STICKY_COL;
	updateStickyCol( viewCursor );
	
	//return something
	return QString::null;
}

QString YZView::moveToStartOfLine( ) {
	return moveToStartOfLine( mainCursor );
}
QString YZView::moveToStartOfLine( YZViewCursor* viewCursor ) {
	//execute the code
	gotoxy(viewCursor, 0 , viewCursor->bufferY());
//	if ( viewCursor == mainCursor )	UPDATE_STICKY_COL;
	updateStickyCol( viewCursor );
	
	//return something
	return QString::null;
}

void YZView::gotoLastLine() {
	unsigned int line = mBuffer->lineCount() - 1;

	if ( getLocalBoolOption("startofline") ) {
		gotoxy(mBuffer->firstNonBlankChar(line), line);
	} else {
		gotoStickyCol( mainCursor, line );
	}
}

QString YZView::gotoLine(const QString& inputsBuff ) {
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
		gotoStickyCol( mainCursor, line );
	}

	return QString::null; //return something
}

QString YZView::moveToEndOfLine( ) {
	return moveToEndOfLine( mainCursor );
}
QString YZView::moveToEndOfLine( YZViewCursor* viewCursor ) {
	gotoxy( viewCursor, mBuffer->textline( viewCursor->bufferY() ).length( ), viewCursor->bufferY());

	stickyCol = STICKY_COL_ENDLINE;
	
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
	updateStickyCol( mainCursor );
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
	updateStickyCol( mainCursor );
}

void YZView::initCopyLine( const YZCursor& pos, unsigned int /*len*/, bool /*applyCursor*/ ) {
}

void YZView::applyCopyLine( const YZCursor& pos, unsigned int /*len*/, bool applyCursor ) {
}

void YZView::initCopyLine( const YZCursor& begin, const YZCursor& end, bool /*applyCursor*/ ) {
}

void YZView::applyCopyLine( const YZCursor& begin, const YZCursor& end, bool applyCursor ) {
}

void YZView::initDeleteLine( const YZCursor& pos, unsigned int /*len*/, bool /*applyCursor*/ ) {
	origPos->setCursor( mainCursor->buffer() );
	gotoxy( 0, pos.getY() ? pos.getY() - 1 : pos.getY(), false );
}

void YZView::applyDeleteLine( const YZCursor& pos, unsigned int /*len*/, bool applyCursor ) {
	paintEvent( dCurrentLeft, mainCursor->screenY(), mColumnsVis, mLinesVis - ( mainCursor->screenY() - dCurrentTop ) );
	if ( applyCursor )
		gotoStickyCol( mainCursor, pos.getY() );
	else
		gotoxy( origPos->getX(), origPos->getY() );
	updateStickyCol( mainCursor );
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
	updateStickyCol( mainCursor );
}

QString YZView::deleteCharacter( unsigned int nb_chars ) {
	mBuffer->action()->deleteChar( this, mainCursor->buffer(), nb_chars );
	return QString::null;
}

void YZView::del(const QString& motion, const QValueList<QChar> &regs) {
	if ( mMode == YZ_VIEW_MODE_VISUAL || mMode == YZ_VIEW_MODE_VISUAL_LINE  ) {
		YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
		selectionPool->clear( "VISUAL" );
		yzDebug() << "Del " << cur_sel.from().getY() << " " << cur_sel.to().getY() << endl;
		mBuffer->action()->deleteArea( this, cur_sel.from(), cur_sel.to(), regs);
		gotoCommandMode();
	} else {
		if ( ! mSession->getMotionPool()->isValid( motion ) ) return; //something's wrong
		//ok we have a motion , so delete till the end of the motion :)
		YZCursor from(mainCursor->buffer()), to(this);
		bool success = mSession->getMotionPool()->applyMotion(motion, this, from, to);
		bool goBack = to < from;
		if ( !success ) {
//			purgeInputBuffer(); //drop me XXX ? should be handle by ::sendKey
			return;
		}
		//delete to the cursor position now :)
		yzDebug() << "Start of motion is : " << from << endl;
		yzDebug() << "End of motion is : " << to << endl;

		if ( goBack ) {
			YZCursor tmp( to );
			to.setCursor( from );
			from.setCursor( tmp );
		}

		mBuffer->action()->deleteArea( this, from, to, regs);
	}
}

QString YZView::deleteLine ( unsigned int nb_lines, const QValueList<QChar> &regs ) {
	QStringList buff; //to copy old lines into the register "
	unsigned int mY = mainCursor->bufferY();
	buff << QString::null;
	for ( unsigned int i = 0; i < nb_lines && ( mY + i ) < ( unsigned int )mBuffer->lineCount(); i++ )
		buff << mBuffer->textline( mY + i );
	buff << QString::null;
	mBuffer->action()->deleteLine( this, mainCursor->buffer(), nb_lines );
	for ( QValueList<QChar>::const_iterator it = regs.begin(); it != regs.end(); it++ ) {
		YZSession::mRegisters.setRegister( *it, buff );
	}
	return QString::null;
}

QString YZView::openNewLineBefore (unsigned int count) {
	for ( unsigned int i = 0 ; i < count ; i++ )
		mBuffer->action()->insertNewLine( this, 0, mainCursor->bufferY() );
	gotoInsertMode();
	return QString::null;
}

QString YZView::openNewLineAfter (unsigned int count) {
	for ( unsigned int i = 0 ; i < count ; i++ )
		mBuffer->action()->insertNewLine( this, 0, mainCursor->bufferY() + 1 );
	gotoInsertMode();
	return QString::null;
}

QString YZView::append () {
	gotoInsertMode();
	gotoxy(mainCursor->bufferX()+1, mainCursor->bufferY() );
	updateStickyCol( mainCursor );

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

QString YZView::gotoCommandMode() {
	mBuffer->undoBuffer()->commitUndoItem(mainCursor->bufferX(), mainCursor->bufferY());
	switchModes(YZ_VIEW_MODE_COMMAND);
	return QString::null;
}

QString YZView::gotoExMode() {
	switchModes(YZ_VIEW_MODE_EX);
	mSession->setFocusCommandLine();
	setCommandLineText( "" );
	return QString::null;
}

QString YZView::gotoOpenMode() {
	switchModes(YZ_VIEW_MODE_OPEN);
	setVisibleArea(80, 1);
	setCommandLineText("");
	yzDebug() << "successfully set open mode" <<endl;
	return QString::null;
}

QString YZView::gotoInsertMode() {
	mBuffer->undoBuffer()->commitUndoItem(mainCursor->bufferX(), mainCursor->bufferY());
	switchModes(YZ_VIEW_MODE_INSERT);
	return QString::null;
}

QString YZView::gotoReplaceMode() {
	mBuffer->undoBuffer()->commitUndoItem(mainCursor->bufferX(), mainCursor->bufferY());
	switchModes(YZ_VIEW_MODE_REPLACE);
	return QString::null;
}

QString YZView::gotoSearchMode( bool reverse ) {
	reverseSearch = reverse;
	switchModes(YZ_VIEW_MODE_SEARCH);
	setCommandLineText( "" );
	return QString::null;
}

QString YZView::gotoVisualMode( bool isVisualLine ) {
	//store the from position
	if ( isVisualLine )
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
	return QString::null;
}

void YZView::leaveVisualMode( ) {
	YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
	selectionPool->clear( "VISUAL" );
	paintEvent( dCurrentLeft, cur_sel.drawFrom().getY() > 0 ? cur_sel.drawFrom().getY() - 1 : 0, mColumnsVis, cur_sel.drawTo().getY() - cur_sel.drawFrom().getY() + 3 );
	gotoCommandMode();
}

YZSelectionMap YZView::getVisualSelection() {
	return selectionPool->layout("VISUAL");
}

void YZView::mark( const QString& mark ) {
	mBuffer->marks()->add( mark , *mainCursor->buffer(), *mainCursor->screen() );
}

void YZView::copy( const QString& motion, const QValueList<QChar> &regs ) {
	if ( mMode == YZ_VIEW_MODE_VISUAL || mMode == YZ_VIEW_MODE_VISUAL_LINE  ) {
		YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
		selectionPool->clear( "VISUAL" );
		mBuffer->action()->copyArea( this, cur_sel.from(), cur_sel.to(), regs);
		gotoCommandMode();
	} else {
		if ( ! mSession->getMotionPool()->isValid( motion ) ) return; //something's wrong
		//ok we have a motion , so delete till the end of the motion :)
		YZCursor from(mainCursor->buffer()), to(this);
		bool success = mSession->getMotionPool()->applyMotion(motion, this, from, to);
		bool goBack = to < from;
		if ( !success ) {
//			purgeInputBuffer(); //drop me XXX ? should be handle by ::sendKey
			return;
		}
		//copy to the cursor position now :)
		yzDebug() << "Start of motion is : " << from << endl;
		yzDebug() << "End of motion is : " << to << endl;

		if ( goBack ) {
			YZCursor tmp( to );
			to.setCursor( from );
			from.setCursor( tmp );
		}

		mBuffer->action()->copyArea( this, from, to, regs);
	}
}

QString YZView::copyLine( unsigned int nb_lines, const QValueList<QChar> &regs ) {
	QStringList list;
	list << QString::null; //just a marker
	for (unsigned int i = 0 ; i < nb_lines; i++ )
		list << mBuffer->textline(mainCursor->bufferY()+i);
	list << QString::null;
	for ( QValueList<QChar>::const_iterator it = regs.begin(); it != regs.end(); it++ )
		YZSession::mRegisters.setRegister( *it, list );
	return QString::null;
}

void YZView::paste( QChar registr, bool after ) {
	QStringList list = YZSession::mRegisters.getRegister( registr );
	if ( list.isEmpty() ) return;

	YZCursor pos( mainCursor->buffer() );
	uint i = 0;
	bool copyWholeLinesOnly = list[ 0 ].isNull();
	QString copy = mBuffer->textline( pos.getY() );
	if ( after || ! copyWholeLinesOnly ) { //paste after current char
		if ( !list[ 0 ].isNull() ) {
			unsigned int start;
			if( after )
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

	} else if ( !after ) { //paste whole lines before current char
		for( i = 1; i < list.size() - 1; i++ ) 
			mBuffer->action()->insertLine( this, pos.getY() + i - 1, list[ i ] );

		gotoxy( pos.getX(), pos.getY() );
	}
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
			updateStickyCol( mainCursor );
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

QString YZView::searchAgain( unsigned int count, bool inverse ) {
	if ( mCurrentSearchItem == 0 ) return QString::null; //no previous search ;)
	if ( inverse ) reverseSearch = !reverseSearch;

	for ( uint i = 0; i < count; i++ )  //search count times
	 	doSearch( mSearchHistory[mCurrentSearchItem-1] );
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

	workCursor->lineHeight = 1;

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
		ret = adjust || workCursor->screenX() + lenToTest - dCurrentLeft <= mColumnsVis - nextLength;

		if ( ret || ! drawMode ) {
			// moving cursors
			workCursor->setScreenX( workCursor->screenX() + workCursor->sColIncrement );
			workCursor->setBufferX( workCursor->bufferX() + workCursor->bColIncrement );
			// update HL
			if ( drawMode ) rHLa += workCursor->bColIncrement;
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
	YzisAttribute hl;
	YzisAttribute * curAt = ( !rHLnoAttribs && (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt ) {
		hl += * curAt;
		return hl.textColor();
	}
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

void YZView::joinLine( unsigned int line, unsigned int count ) {
	if ( line >= mBuffer->lineCount() - 1 ) return;
	gotoxy( mBuffer->textline( line ).length() - 1, line );
	updateStickyCol( mainCursor );
	for ( unsigned int i = 0; i < count ; i ++ ) 
		mBuffer->mergeNextLine( line );
	paintEvent( dCurrentLeft, mainCursor->screenY(), mColumnsVis, mLinesVis - ( mainCursor->screenY() - dCurrentTop ) );
}

void YZView::printToFile( const QString& path ) {
	YZPrinter printer( this );
	printer.printToFile( path );
	printer.run( );
}

void YZView::undo( unsigned int count ) {
	for ( unsigned int i = 0 ; i < count ; i++ )
		mBuffer->undoBuffer()->undo( this );
}

void YZView::redo( unsigned int count ) {
	for ( unsigned int i = 0 ; i < count ; i++ )
		mBuffer->undoBuffer()->redo( this );
}

QString YZView::match( const QString& ) {
	bool found = false;
	YZCursor pos = mBuffer->action()->match( this, *mainCursor->buffer(), &found );
	yzDebug() << "Result " << pos << endl;

	if ( found )
		gotoxy( pos.getX(), pos.getY() );

	return QString::null;
}

void YZView::gotoMark( const QString& mark) {
	yzDebug() << "gotoMark " << mark << endl;
	bool found = false;
	YZCursorPos pos = mBuffer->marks()->get(mark, &found);
	if ( found )
		gotoxy(pos.bPos->getX(), pos.bPos->getY());
}

QString YZView::refreshScreenInternal() {
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

void YZView::gotoStickyCol( YZViewCursor* viewCursor, unsigned int Y) {
	if ( stickyCol == STICKY_COL_ENDLINE ) gotoxy( viewCursor, mBuffer->textline( Y ).length(), Y );
	else gotodxy( viewCursor, stickyCol, Y );
}
void YZView::updateStickyCol( YZViewCursor* viewCursor ) {
	stickyCol = viewCursor->screenX();
}

void YZView::commitNextUndo() {
	mBuffer->undoBuffer()->commitUndoItem( mainCursor->bufferX(), mainCursor->bufferY() );
}

YZCursor *YZView::getCursor() {
	return mainCursor->screen();
}

YZCursor *YZView::getBufferCursor() {
	return mainCursor->buffer();
}
