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

#define STICKY_COL_ENDLINE -1

static const QChar tabChar( '\t' );
static QColor fake;

YZView::YZView(YZBuffer *_b, YZSession *sess, int lines) {
	myId = YZSession::mNbViews++;
	yzDebug() << "New View created with UID : " << myId << endl;
	YZASSERT( _b ); YZASSERT( sess );
	mSession = sess;
	mBuffer	= _b;
	mLinesVis = lines;
	mCursor = new YZCursor(this);
	dCursor = new YZCursor(this);
	sCursor = new YZCursor(this);
	rCursor = new YZCursor(this);
	origPos = new YZCursor(this);

	/* start of visual mode */
	mVisualCursor = new YZCursor(this);
	dVisualCursor = new YZCursor(this);

	dSpaceFill = 0;
	dColLength = 0;
	mColLength = 0;
	dLineLength = 1;
	mLineLength = 1;
	dWrapNextLine = false;

	stickyCol = 0;

	mMaxX = 0;
	mMode = YZ_VIEW_MODE_COMMAND;
	mCurrentLeft = mCursor->getX();
	mCurrentTop = mCursor->getY();
	dCurrentLeft = dCursor->getX();
	dCurrentTop = dCursor->getY();

	QString line = mBuffer->textline(mCurrentTop);
	if (!line.isNull()) mMaxX = line.length()-1;

	mCurrentExItem = 0;
	mCurrentSearchItem = 0;
	mExHistory.resize(200);
	mSearchHistory.resize(200);
	reverseSearch=false;
	viewInformation.l = viewInformation.c1 = viewInformation.c2 = 0;
	viewInformation.percentage = "";

	selectionPool = new YZSelectionPool( this );
}

YZView::~YZView() {
	delete selectionPool;
	yzDebug() << "Deleting view " << myId << endl;
	mBuffer->rmView(this); //make my buffer forget about me
}

void YZView::setVisibleArea(int c, int l, bool refresh) {
	yzDebug() << "YZView::setVisibleArea(" << c << "," << l << ");" << endl;
	mLinesVis = l;
	mColumnsVis = c;
	if( refresh ) {
		refreshScreen();
		gotoxy( mCursor->getX(), mCursor->getY() );
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

/* Used by the buffer to post events */
void YZView::sendKey( int c, int modifiers) {
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
	yzDebug() << "MappedCommand " << mappedCommand << endl;

	//ignore some keys
	if ( c == Qt::Key_Shift || c == Qt::Key_Alt || c == Qt::Key_Meta ||c == Qt::Key_Control || c == Qt::Key_CapsLock ) {
		yzError( )<< "receiving modifiers in c, shouldn't happen" <<endl;
		return;
	}

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
					if (mCursor->getX() > 0) moveLeft();
					gotoPreviousMode();
					return;
				case Qt::Key_Return:
					test = mCursor->getX() == 0;
					mBuffer->action()->insertNewLine( this, mCursor );
					if ( test ) {
						gotoxy( 0, mCursor->getY() + 1 );
						stickyCol = dCursor->getX( );
					}
					return;
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
					if (mCursor->getX() == 0 && mCursor->getY() > 0 && YZSession::getStringOption( "General\\backspace" ).contains( "eol" ) ) {
						joinLine( mCursor->getY() - 1 );
						if ( mCursor->getX() ) moveRight();
					} else if ( mCursor->getX() > 0 )
						mBuffer->action()->deleteChar( this, mCursor->getX() - 1, mCursor->getY(), 1 );
					return;
				case Qt::Key_Delete:
					mBuffer->action()->deleteChar( this, mCursor, 1 );
					return;
				case Qt::Key_PageDown:
					gotodxy(dCursor->getX(), mCursor->getY() + mLinesVis );
					purgeInputBuffer();
					return;
				case Qt::Key_PageUp:
					gotodxy(dCursor->getX(), ( mCursor->getY() > mLinesVis ? mCursor->getY() - mLinesVis : 0 ) );
					purgeInputBuffer();
					return;
				case Qt::Key_Tab:
					key = "\t";
				default:
					mBuffer->action()->insertChar( this, mCursor, key );
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
					mBuffer->action()->deleteChar( this, mCursor, 1 );
					return;
				case Qt::Key_Escape:
					if ( mCursor->getX() == mBuffer->textline( mCursor->getY() ).length() ) 
						moveToEndOfLine( );
					gotoPreviousMode();
					return;
				case Qt::Key_Return:
					test = mCursor->getX() == 0;
					mBuffer->action()->insertNewLine( this, mCursor );
					if ( test ) {
						gotoxy( 0, mCursor->getY() + 1 );
						stickyCol = dCursor->getX( );
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
					mBuffer->action()->replaceChar( this, mCursor, "\t" );
					return;
				case Qt::Key_PageDown:
					gotodxy(dCursor->getX(), mCursor->getY() + mLinesVis );
					purgeInputBuffer();
					return;
				case Qt::Key_PageUp:
					gotodxy(dCursor->getX(), ( mCursor->getY() > mLinesVis ? mCursor->getY() - mLinesVis : 0 ) );
					purgeInputBuffer();
					return;
				default:
					mBuffer->action()->replaceChar( this, mCursor, key );
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
				case Qt::Key_Return:
					yzDebug() << "Current command EX : " << getCommandLineText();
					if(getCommandLineText().isEmpty())
						return;

					mExHistory[mCurrentExItem] = getCommandLineText();
					mCurrentExItem++;
					mSession->getExPool()->execExCommand( this, getCommandLineText() );
					setCommandLineText( "" );
					mSession->setFocusMainWindow();
					gotoPreviousMode();
					return;
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
			switch ( c ) {
				case Qt::Key_Escape:
					YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
					selectionPool->clear( "VISUAL" );
					paintEvent( dCurrentLeft, cur_sel.drawFrom->getY(), mColumnsVis, cur_sel.drawTo->getY() - cur_sel.drawFrom->getY() + 1 );
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
					if ( mMode == YZ_VIEW_MODE_VISUAL ) {
						YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
						selectionPool->clear( "VISUAL" );
						mBuffer->action()->deleteArea( this, *cur_sel.from, *cur_sel.to, QChar( '\"' ));
						purgeInputBuffer();
						gotoCommandMode();
						return;
					} else
						mBuffer->action()->deleteChar( this, *mCursor, 1);
					return;
				case Qt::Key_PageDown:
					gotodxy(mCursor->getX(), mCursor->getY() + mLinesVis );
					purgeInputBuffer();
					return;
				case Qt::Key_PageUp:
					gotodxy(dCursor->getX(), ( mCursor->getY() > mLinesVis ? mCursor->getY() - mLinesVis : 0 ) );
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

void YZView::updateCursor() {
	static unsigned int lasty = 1<<31; // small speed optimisation
	viewInformation.percentage = tr( "All" );
	unsigned int y = mCursor->getY();

	if ( y != lasty ) {
		unsigned int nblines = mBuffer->lineCount();
		viewInformation.percentage = QString("%1%").arg( ( unsigned int )( y*100/ ( nblines==0 ? 1 : nblines )));
		if ( mCurrentTop < 1 )  viewInformation.percentage=tr( "Top" );
		if ( mCurrentTop+mLinesVis >= nblines )  viewInformation.percentage=tr( "Bot" );
		if ( (mCurrentTop<1 ) &&  ( mCurrentTop+mLinesVis >= nblines ) ) viewInformation.percentage=tr( "All" );
		lasty=y;
	}

	viewInformation.l = y;
	viewInformation.c1 = mCursor->getX(); 
	viewInformation.c2 = dCursor->getX(); // XXX pas du tout, c'est c1 mais en remplacant les tabs par 'tablenght' <-- avec le QRegexp() mais je l'ai perdu

	syncViewInfo();
}

void YZView::centerViewHorizontally(unsigned int column) {
//	yzDebug() << "YZView::centerViewHorizontally " << column << endl;
	unsigned int newcurrentLeft = 0;
	if ( column > mColumnsVis/2 ) newcurrentLeft = column - mColumnsVis / 2;

	if (newcurrentLeft > 0) {
		initGoto( );
		gotoy( mCursor->getY( ) );
		gotodx( newcurrentLeft );
		dCurrentLeft = rCursor->getX( );
		mCurrentLeft = sCursor->getX( );
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
	if ( wrap && newcurrent > 0 ) {
		initDraw( );
		drawMode = false;
		gotody( newcurrent );
		newcurrent = sCursor->getY() + ( !alignTop && rLineHeight > 1 ? 1 : 0 );
		gotoy( newcurrent );
		dCurrentTop = rCursor->getY( );
		mCurrentTop = sCursor->getY( );
	} else {
		dCurrentTop = newcurrent;
		mCurrentTop = newcurrent;
	}
	if ( old_dCurrentTop > dCurrentTop && old_dCurrentTop - dCurrentTop < mLinesVis ) {
		scrollUp( old_dCurrentTop - dCurrentTop );
	} else if ( old_dCurrentTop < dCurrentTop && dCurrentTop - old_dCurrentTop < mLinesVis ) {
		scrollDown( dCurrentTop - old_dCurrentTop );
	} else {
		 refreshScreen();
	}
	if ( alignTop ) 
		gotoxy( mCurrentLeft, mCurrentTop );
}

/* recalculate cursor position + refresh screen */
void YZView::reset( ) {
	initDraw( 0, 0, 0, 0 );
	drawMode = false;
	gotoy( mCursor->getY() );
	gotox( mCursor->getX() );
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
	else if ( sCursor->getX() >= sCurLineLength ) {
		gotox ( sCurLineLength );
		return;
	}
	while ( rCursor->getX() > nextx )
		if ( ! drawPrevCol( ) ) break;
	while ( rCursor->getX() < nextx && sCursor->getX() < sCurLineLength - shift )
		drawNextCol( );
}

void YZView::gotox( unsigned int nextx ) {
	if ( ( int )nextx < 0 ) nextx = 0;
	unsigned int shift = ( ! drawMode && ( YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT==mMode && sCurLineLength > 0 ) ) ? 1 : 0;
	if ( nextx >= sCurLineLength ) {
		if ( sCurLineLength == 0 ) nextx = 0;
		else nextx = sCurLineLength - 1 + shift;
	}
	while ( sCursor->getX() > nextx ) {
		if ( ! wrap || rCurLineLength <= mColumnsVis ) {
			if ( ! drawPrevCol( ) ) break;
		} else {
			if ( ! drawPrevCol( ) ) {
				if ( sCursor->getX() >= nextx && wrapNextLine ) drawPrevLine( );
				else break;
			}
		}
	}
	while ( sCursor->getX() < nextx ) {
		if ( ! wrap || rCurLineLength <= mColumnsVis ) {
			drawNextCol( );
		} else {
			while ( drawNextCol() && sCursor->getX() < nextx ) ;
			if ( wrapNextLine ) drawNextLine( );
		}
	}
}

void YZView::gotody( unsigned int nexty ) {
	if ( ( int )nexty < 0 ) nexty = 0;
	if ( sCursor->getY() >= mBuffer->lineCount() ) nexty = mBuffer->lineCount() - 1;

	/* some easy case */
	if ( nexty == 0 ) {
		gotoy( 0 );
	} else if ( nexty == dCurrentTop ) {
		gotoy( mCurrentTop );
	} else {
		if ( wrap ) gotoy( mCurrentTop );
		while ( rCursor->getY() > nexty )
			drawPrevLine( );
		while ( rCursor->getY() < nexty && sCursor->getY() < mBuffer->lineCount() - 1 ) {
			if ( wrap && ! wrapNextLine && rCurLineLength > mColumnsVis ) 
				while( drawNextCol( ) ) ;
			drawNextLine( );
			if ( wrap && rCursor->getY() < nexty && rCurLineLength > mColumnsVis )
				while ( drawNextCol( ) ) ;
		}
	}
}

void YZView::gotoy( unsigned int nexty ) {
	if ( ( int )nexty < 0 ) nexty = 0;
	if ( nexty >= mBuffer->lineCount() ) nexty = mBuffer->lineCount() - 1;

	/* some easy case */
	if ( nexty == 0 ) {
		initDraw( 0, 0, 0, 0 );
		drawMode = false;
		rLineHeight = rLineLength = sLineLength = 1;
	} else if ( nexty == mCurrentTop ) {
		initDraw( );
		drawMode = false;
		rLineHeight = rLineLength = sLineLength = 1;
	} else {
		while ( sCursor->getY() > nexty ) {
			drawPrevLine( );
			if ( wrap && sCursor->getY() == nexty && rCurLineLength > mColumnsVis ) { 
				/* goto begin of line */
				unsigned int wrapLineMinHeight = ( unsigned int ) ceil( sCurLineLength / mColumnsVis ) + 1;
				unsigned int wrapLineMaxHeight = ( unsigned int ) ceil( rCurLineLength / mColumnsVis ) + 1;
				if ( wrapLineMinHeight == wrapLineMaxHeight ) {
					rCursor->setY( rCursor->getY() + 1 - wrapLineMinHeight );
				} else {
					unsigned int prevRX = rCursor->getY();
					initDraw( 0, nexty, 0, 0 );
					while ( drawNextCol( ) ) ;
					while ( sCursor->getY() == nexty ) {
						wrapLineMinHeight = rLineHeight;
						drawNextLine( );
						if ( sCursor->getY() == nexty ) while ( drawNextCol( ) ) ;
					}
					initDraw ( 0, nexty, 0, prevRX - wrapLineMinHeight );
					rLineHeight = rLineLength = sLineLength = 1;
				}
			}
		} 
		while ( sCursor->getY() < nexty ) {
			if ( wrap && ! wrapNextLine && rCurLineLength > mColumnsVis ) 
				while( drawNextCol( ) ) ;
			drawNextLine( );
			if ( wrap && sCursor->getY() < nexty && rCurLineLength > mColumnsVis )
				while ( drawNextCol( ) ) ;
		}
	}
}

void YZView::initGoto( ) {
	initDraw( mCursor->getX(), mCursor->getY(), dCursor->getX(), dCursor->getY() );
	rSpaceFill = dSpaceFill;
	rColLength = dColLength;
	sColLength = mColLength;
	rLineLength = dLineLength;
	rLineHeight = dLineHeight;
	sLineLength = mLineLength;
	wrapNextLine = dWrapNextLine;
	drawMode = false;
}

void YZView::applyGoto( bool applyCursor ) {
	/* draw members -> cursor members */
	dCursor->setX( rCursor->getX() );
	dCursor->setY( rCursor->getY() );
	mCursor->setX( sCursor->getX() );
	mCursor->setY( sCursor->getY() );

	dSpaceFill = rSpaceFill;
	dColLength = rColLength;
	dLineLength = rLineLength;
	mColLength = sColLength;
	mLineLength = sLineLength;
	dWrapNextLine = wrapNextLine;
	dLineHeight = rLineHeight;

/*	yzDebug() << "applyGoto : "
			<< "dColLength=" << dColLength << "; dSpaceFill=" << dSpaceFill
			<< "; dLineLength=" << dLineLength << "; mLineLength=" << mLineLength
			<< "; dWrapNextLine=" << dWrapNextLine
			<< endl; 
	yzDebug( ) << "mCursor:" << mCursor->getX( ) << "," << mCursor->getY( )<< "; dCursor:" << dCursor->getX( ) << "," << dCursor->getY( ) << endl; */
	

	if ( applyCursor ) {

		if ( mMode == YZ_VIEW_MODE_VISUAL ) {

			YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
			/* erase current selection */
			selectionPool->clear( "VISUAL" );

			if ( *mCursor > *mVisualCursor )
				selectionPool->addSelection( "VISUAL", *mVisualCursor, *mCursor, *dVisualCursor, *dCursor );
			else
				selectionPool->addSelection( "VISUAL", *mCursor, *mVisualCursor, *dCursor, *dVisualCursor );

			/* apply new selection */
			YZSelection new_cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
			if ( cur_sel.drawFrom != NULL && *new_cur_sel.drawFrom > *cur_sel.drawFrom ) *new_cur_sel.drawFrom = *cur_sel.drawFrom;
			if ( cur_sel.drawTo != NULL && *new_cur_sel.drawTo < *cur_sel.drawTo ) *new_cur_sel.drawTo = *cur_sel.drawTo;
			paintEvent( dCurrentLeft, new_cur_sel.drawFrom->getY(), mColumnsVis, new_cur_sel.drawTo->getY() - new_cur_sel.drawFrom->getY() + 1 );
		}

		if ( !isLineVisible( dCursor->getY() ) )
			alignViewVertically( dCursor->getY( ) );
		if ( !isColumnVisible( dCursor->getX(), dCursor->getY() ) ) {
			centerViewHorizontally( dCursor->getX( ) );
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
	unsigned int nextLine = QMIN( mCursor->getY() + nb_lines, mBuffer->lineCount()-1);
	if ( stickyCol == STICKY_COL_ENDLINE )
		gotoxy( mBuffer->textline( nextLine ).length(), nextLine );
	else
		gotodxy( stickyCol, nextLine );

	//reset the input buffer
	purgeInputBuffer();

	//return something
	return QString::null;
}

QString YZView::moveUp( const QString& , YZCommandArgs args ) {
	unsigned int nb_lines=args.count;

	//execute the code
	unsigned int nextLine = QMAX( mCursor->getY()-nb_lines,0);
	if ( stickyCol == STICKY_COL_ENDLINE )
		gotoxy( mBuffer->textline( nextLine ).length(), nextLine );
	else
		gotodxy( stickyCol, nextLine );

	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveLeft( const QString& , YZCommandArgs args ) {
	uint nb_cols=args.count;

	//execute the code
	nb_cols = QMIN( mCursor->getX(), nb_cols );
	gotoxy( mCursor->getX() ? mCursor->getX() - nb_cols : 0 , mCursor->getY());

	stickyCol = dCursor->getX( );

	//reset the input buffer
	purgeInputBuffer();

	//return something
	return QString::null;
}

QString YZView::moveRight( const QString& , YZCommandArgs args ) {
	int nb_cols=args.count;
	
	//execute the code
	gotoxy(mCursor->getX() + nb_cols , mCursor->getY());

	stickyCol = dCursor->getX( );
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveToFirstNonBlankOfLine( const QString&, YZCommandArgs ) {
	//execute the code
	gotoxy(mBuffer->firstNonBlankChar(mCursor->getY()) , mCursor->getY());
	stickyCol = dCursor->getX( );
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveToStartOfLine( const QString&, YZCommandArgs ) {
	//execute the code
	gotoxy(0 , mCursor->getY());
	stickyCol = dCursor->getX( );
	
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

	if ( YZSession::getBoolOption("General\\startofline") ) {
		gotoxy(mBuffer->firstNonBlankChar(line), line);
	} else {
		if ( stickyCol == STICKY_COL_ENDLINE )
			gotoxy( mBuffer->textline( line ).length(), line );
		else
			gotodxy( stickyCol, line );
	}

	purgeInputBuffer();
	return QString::null; //return something
}

// end of goto-like command


QString YZView::moveToEndOfLine( const QString&, YZCommandArgs ) {
	gotoxy( mBuffer->textline( mCursor->getY() ).length( ), mCursor->getY());

	stickyCol = STICKY_COL_ENDLINE;
	
	purgeInputBuffer();
	return QString::null;
}

/*
 * ACTIONS
 */

void YZView::initChanges( const YZCursor& pos ) {
	origPos->setCursor( mCursor );
	if ( wrap ) {
		gotoxy( mBuffer->textline( pos.getY() ).length(), pos.getY(), false );
		lineDY = dCursor->getY();
	}
	gotoxy( pos.getX(), pos.getY(), false );
}

void YZView::applyChanges( const YZCursor& pos, unsigned int len, bool applyCursor ) {
	unsigned int dY = dCursor->getY();
	if ( wrap ) {
		gotoxy( mBuffer->textline( pos.getY() ).length(), pos.getY(), false );
		if ( dCursor->getY() != lineDY )
			paintEvent( dCurrentLeft, dY, mColumnsVis, mLinesVis - ( dY - dCurrentTop ) );
		else
			paintEvent( dCurrentLeft, dY, mColumnsVis, 1 + dCursor->getY() - dY );
	} else 
		paintEvent( dCurrentLeft, dY, mColumnsVis, 1 );

	if ( applyCursor ) 
		gotoxy( pos.getX() + len, pos.getY() );
	else 
		gotoxy( origPos->getX(), origPos->getY(), false );
	stickyCol = dCursor->getX( );
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

void YZView::initInsertLine( const YZCursor& pos, bool /*applyCursor*/ ) {
	origPos->setCursor( mCursor );
	gotoxy( pos.getX(), pos.getY(), false );
}

void YZView::applyInsertLine( const YZCursor& pos, bool applyCursor ) { 
	paintEvent( dCurrentLeft, dCursor->getY(), mColumnsVis, mLinesVis - ( dCursor->getY() - dCurrentTop ) );
	if ( applyCursor )
		gotoxy( 0, pos.getY() + ( pos.getX() ? 1 : 0 ) );
	else
		gotoxy( origPos->getX(), origPos->getY() );
	stickyCol = dCursor->getX( );
}

void YZView::initDeleteLine( const YZCursor& pos, unsigned int /*len*/, bool /*applyCursor*/ ) {
	origPos->setCursor( mCursor );
	gotoxy( 0, pos.getY() ? pos.getY() - 1 : pos.getY(), false );
}

void YZView::applyDeleteLine( const YZCursor& pos, unsigned int /*len*/, bool applyCursor ) {
	paintEvent( dCurrentLeft, dCursor->getY(), mColumnsVis, mLinesVis - ( dCursor->getY() - dCurrentTop ) );
	if ( applyCursor )
		gotoxy( 0, pos.getY() );
	else
		gotoxy( origPos->getX(), origPos->getY() );
	stickyCol = dCursor->getX( );
}

void YZView::initDeleteLine( const YZCursor& begin, const YZCursor& end, bool /*applyCursor*/ ) {
	origPos->setCursor( mCursor );
	if ( wrap && begin.getY() == end.getY() ) {
		gotoxy( mBuffer->textline( begin.getY() ).length(), begin.getY(), false );
		lineDY = dCursor->getY();
	}
	gotoxy( begin.getX() ? begin.getX() - 1 : 0, begin.getY(), false );
}

void YZView::applyDeleteLine( const YZCursor& begin, const YZCursor& end, bool applyCursor ) {
	if ( begin.getY() == end.getY() )
		paintEvent( dCurrentLeft, dCursor->getY(), mColumnsVis, mLinesVis - ( dCursor->getY() - dCurrentTop ) );
	else
		applyDeleteLine( begin, 1, applyCursor );
	if ( applyCursor )
		gotoxy( begin.getX(), begin.getY() );
	else
		gotoxy( origPos->getX(), origPos->getY() );
	stickyCol = dCursor->getX( );
}



QString YZView::deleteCharacter( const QString& , YZCommandArgs args ) {
	mBuffer->action()->deleteChar( this, mCursor, args.count );
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
	unsigned int mY = mCursor->getY();
	unsigned int mX = mCursor->getX();
	if ( args.command == "dd" ) { //delete whole lines
		for ( int i = 0; i < nb_lines && ( mY + i ) < ( unsigned int )mBuffer->lineCount(); i++ )
			buff << mBuffer->textline( mY + i );
		mBuffer->action()->deleteLine( this, mCursor, nb_lines );
		YZSession::mRegisters.setRegister( reg, buff );
		purgeInputBuffer();
		return QString::null;
	} else if ( args.command == "D" ) { //delete to end of lines
		//just fake the real command
		args.command = "d$";
		args.motion = "$";
	} 
	if ( mMode == YZ_VIEW_MODE_VISUAL && args.command.startsWith( "d" ) ) {
		YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
		selectionPool->clear( "VISUAL" );
		mBuffer->action()->deleteArea( this, *cur_sel.from, *cur_sel.to, reg);
		gotoCommandMode();
	} else if (args.command.startsWith("d")) {
		if ( ! mSession->getMotionPool()->isValid( args.motion ) ) return QString::null; //keep going waiting for new inputs
		//ok we have a motion , so delete till the end of the motion :)
		YZCursor cursor(this);
		bool goBack = false;
		bool success = mSession->getMotionPool()->applyMotion(args.motion, this, &goBack, cursor);
		if ( !success ) {
			purgeInputBuffer();
			return QString::null;
		}
		//delete to the cursor position now :)
		yzDebug() << "Start of motion is : " << *mCursor << endl;
		yzDebug() << "End of motion is : " << cursor << endl;

		if ( goBack ) {
			mY = cursor.getY();
			mX = cursor.getX();
			cursor.setCursor( mCursor );
			gotoxy( mX, mY, false );
		}

		mBuffer->action()->deleteArea( this, *mCursor, cursor, reg );
	}
	//reset the input buffer
	purgeInputBuffer();

	return QString::null;
}


QString YZView::openNewLineBefore ( const QString&, YZCommandArgs ) {
	mBuffer->action()->insertNewLine( this, 0, mCursor->getY() );
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();
	return QString::null;
}

QString YZView::openNewLineAfter ( const QString&, YZCommandArgs ) {
	mBuffer->action()->insertNewLine( this, 0, mCursor->getY() + 1 );
	purgeInputBuffer();
	gotoInsertMode();
	return QString::null;
}

QString YZView::append ( const QString&, YZCommandArgs ) {
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();
	gotoxy(mCursor->getX()+1, mCursor->getY() );
	stickyCol = dCursor->getX( );

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
	mBuffer->undoBuffer()->commitUndoItem(mCursor->getX(), mCursor->getY());
	switchModes(YZ_VIEW_MODE_COMMAND);
	setVisibleArea(80, 24);
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
	mBuffer->undoBuffer()->commitUndoItem(mCursor->getX(), mCursor->getY());
	switchModes(YZ_VIEW_MODE_INSERT);
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoReplaceMode(const QString&, YZCommandArgs ) {
	mBuffer->undoBuffer()->commitUndoItem(mCursor->getX(), mCursor->getY());
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

QString YZView::gotoVisualMode(const QString&, YZCommandArgs ) {
	//store the from position
	*mVisualCursor = *mCursor;
	*dVisualCursor = *dCursor;
	selectionPool->clear( "VISUAL" );
	selectionPool->addSelection( "VISUAL", *mVisualCursor, *mVisualCursor, *dVisualCursor, *dVisualCursor );
	yzDebug("Visual mode") << "Starting at " << *mCursor << endl;
	switchModes(YZ_VIEW_MODE_VISUAL);
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
			list << mBuffer->textline(mCursor->getY()+i);
		list << QString::null;
	} else if ( args.command == "Y" ) {
		list << QString::null; //just a marker
		for (int i = 0 ; i < nb_lines; i++ )
			list << mBuffer->textline(mCursor->getY()+i);
		list << QString::null;
	} else if ( args.command == "y$" ) {
		QString lin = mBuffer->textline( mCursor->getY() );
		list << lin.mid(mCursor->getX());
	} else if ( mMode == YZ_VIEW_MODE_VISUAL && args.command.startsWith( "y" ) ) {
		YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
		selectionPool->clear( "VISUAL" );
		list = mBuffer->getText( *cur_sel.from, *cur_sel.to );
		paintEvent( dCurrentLeft, cur_sel.drawFrom->getY(), mColumnsVis, cur_sel.drawTo->getY() - cur_sel.drawFrom->getY() + 1 );
		gotoCommandMode();
	} else if ( args.command.startsWith( "y" ) ) {
		if ( ! mSession->getMotionPool()->isValid( args.motion ) ) return QString::null; //keep going waiting for new inputs
		//ok we have a motion , so delete till the end of the motion :)
		YZCursor cursor(this);
		bool goBack = false;
		bool success = mSession->getMotionPool()->applyMotion(args.motion, this, &goBack, cursor);
		if ( !success ) {
			purgeInputBuffer();
			return QString::null;
		}
		//copy to the cursor position now :)
		yzDebug() << "Start of motion is : " << *mCursor << endl;
		yzDebug() << "End of motion is : " << cursor << endl;

		if ( goBack ) {
			int mY = cursor.getY();
			int mX = cursor.getX();
			cursor.setCursor( mCursor );
			gotoxy( mX, mY, false );
		}

		list = mBuffer->getText(*mCursor, cursor);
	}
	
	YZSession::mRegisters.setRegister( args.registr, list );

	purgeInputBuffer();
	return QString::null;
}

QString YZView::paste( const QString& , YZCommandArgs args ) {
	QStringList list = YZSession::mRegisters.getRegister( args.registr );

	YZCursor pos( mCursor );

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
	int currentMatchLine = mCursor->getY(); //start from current line
	//if you understand this line you are semi-god :)
	//if you don't understand, remove the IFs and see what it does when you have the cursor on the last/first column and start a search/reverse search
	int currentMatchColumn = reverseSearch ? ( mCursor->getX() ? mCursor->getX() : 1 ) - 1 : ( mCursor->getX() < mMaxX ) ? mCursor->getX() + 1 : mCursor->getX(); //start from current column +/- 1

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
			if ( mCursor->getX() == ( unsigned int ) idx ) { //ok we did not move guy (col 0 or last col maybe ...)
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
	for ( uint i = 0; i < args.count; i++ )  //search count times
	 	doSearch( mSearchHistory[mCurrentSearchItem-1] );
	purgeInputBuffer();
	return QString::null;
}

bool YZView::isColumnVisible( unsigned int column, unsigned int  ) {
	return ! (column < dCurrentLeft || column >= (dCurrentLeft + mColumnsVis));
}

/* update sCurLine informations */
void YZView::updateCurLine( ) {
	sCurLineLength = sCurLine.length();
	if ( wrap ) rCurLineLength = sCurLineLength + sCurLine.contains( '\t' ) * ( tabwidth - 1 );
}
	

void YZView::initDraw( ) {
	initDraw( mCurrentLeft, mCurrentTop, dCurrentLeft, dCurrentTop );
}

void YZView::initDraw( unsigned int sLeft, unsigned int sTop, 
			unsigned int rLeft, unsigned int rTop ) {
	sCurrentLeft = sLeft;
	sCurrentTop = sTop;
	rCurrentLeft = rLeft;
	rCurrentTop = rTop;

	sCursor->setX( sCurrentLeft );
	sCursor->setY( sCurrentTop );
	rCursor->setX( rCurrentLeft );
	rCursor->setY( rCurrentTop );

	rLineLength = 0;
	rColLength = 0;
	sLineLength = 0;
	sColLength = 0;
	rLineHeight = 0;
	rSpaceFill = 0;

	tabwidth = YZSession::getIntOption("General\\tabwidth");
	wrap = YZSession::getBoolOption( "General\\wrap" );

	wrapNextLine = false;
	if ( sCursor->getY() < mBuffer->lineCount() && ! mBuffer->textline( sCursor->getY() ).isNull() )
		sCurLine = mBuffer->textline ( sCursor->getY() );
	else
		sCurLine = "";
	updateCurLine( );

	drawMode = true;
}

bool YZView::drawPrevLine( ) {
	if ( ! wrapNextLine ) {
		sCursor->setX( sCurrentLeft );
		rCursor->setX( rCurrentLeft );
		sCursor->setY( sCursor->getY() - sLineLength );
		rSpaceFill = 0;
		sLineLength = 1;
		sColLength = 0;
		rColLength = 0;
		rLineHeight = 1;
	} else {
		rCursor->setX( mColumnsVis - rColLength );
		rSpaceFill -= ( tabwidth - mColumnsVis % tabwidth ) % tabwidth;
		--rLineHeight;
	}
	rCursor->setY( rCursor->getY( ) - rLineLength );
	rLineLength = 1;

	if ( sCursor->getY() < mBuffer->lineCount() ) {
		YZLine *yl = mBuffer->yzline( sCursor->getY() );
		if ( ! wrapNextLine ) {
			sCurLine = yl->data();
			updateCurLine( );
		}
		if ( rCurrentLeft > 0 && ! wrapNextLine ) {
			sCursor->setX( 0 );
			rCursor->setX( 0 );
			gotodx( rCurrentLeft );
			if ( drawMode ) {
				rSpaceFill -= ( tabwidth - rCurrentLeft % tabwidth ) % tabwidth;
				if ( rCursor->getX( ) > rCurrentLeft ) {
					sCursor->setX( sCursor->getX() - 1 );
				}
				rColLength = 0;
				sColLength = 0;
			}
		}
		if ( ( rCursor->getY() - rCurrentTop ) < mLinesVis ) {
			return true;
		}
	} else {
		sCurLine = "";
		sCurLineLength = sCurLine.length();
	}
	wrapNextLine = false;
	return false;
}

bool YZView::drawNextLine( ) {
	if ( ! wrapNextLine ) {
		sCursor->setX( sCurrentLeft );
		sCursor->setY( sCursor->getY() + sLineLength );
		rCursor->setX( rCurrentLeft );
		rSpaceFill = 0;
		sLineLength = 1;
		rLineHeight = 1;
	} else {
		if ( drawMode )
			sCursor->setX( sCursor->getX() - 1 - ( rCursor->getX() - mColumnsVis > 1 ? 1 : 0 ) );
		else if ( ! drawMode )
			sCursor->setX( sCursor->getX() - ( rCursor->getX() - mColumnsVis > 0 ? 1 : 0 ) );
		rCursor->setX( 0 );
		rSpaceFill += ( tabwidth - mColumnsVis % tabwidth ) % tabwidth;
		++rLineHeight;
	}
	rCursor->setY( rCursor->getY( ) + rLineLength );
	rLineLength = 1;
	sColLength = 0;
	rColLength = 0;

	if ( sCursor->getY() < mBuffer->lineCount() ) {
		YZLine *yl = mBuffer->yzline( sCursor->getY() );
		if ( ! wrapNextLine ) {
			sCurLine = yl->data();
			updateCurLine( );
		}
		if ( rCurrentLeft > 0 && ! wrapNextLine ) {
			sCursor->setX( 0 );
			rCursor->setX( 0 );
			gotodx( rCurrentLeft );
			if ( drawMode ) {
				rSpaceFill += ( tabwidth - rCurrentLeft % tabwidth ) % tabwidth;
				if ( rCursor->getX( ) > rCurrentLeft ) {
					sCursor->setX( sCursor->getX() - 1 );
					rCursor->setX( rCurrentLeft );
				}
			}
			rColLength = 0;
			sColLength = 0;
		}
		if ( drawMode && ( rCursor->getY() - rCurrentTop ) < mLinesVis ) {
			rHLa = NULL;
			if ( yl->length() != 0 )
				rHLa = yl->attributes();
			rHLnoAttribs = !rHLa;
			rHLa = rHLa + sCursor->getX( ) - 1;
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
	wrapNextLine = false;
	return false;
}

bool YZView::drawPrevCol( ) {
	wrapNextLine = false;
	if ( sCursor->getX() >= sColLength ) {
		unsigned int curx = sCursor->getX( ) - 1;
		sCursor->setX( curx );
		lastChar = sCurLine[ curx ];
		if ( lastChar != tabChar ) {
			rColLength = 1;
			sColLength = 1;
			if ( rCursor->getX() >= rColLength )
				rCursor->setX( rCursor->getX( ) - rColLength );
			else
				wrapNextLine = ( wrap && rCurLineLength > mColumnsVis && rCursor->getX() == 0 && sCursor->getX() > 0 );
			sLineLength = wrapNextLine ? 0 : 1;
		} else {
			/* go back to begin of line */
			initDraw( 0, sCursor->getY(), 0, rCursor->getY() - ( rLineHeight - 1 ) );
			rLineLength = 1;
			rLineHeight = 1;
			return false;
		}
	}
	return ! wrapNextLine;
}

bool YZView::drawNextCol( ) {
	bool ret = false;
	unsigned int curx = sCursor->getX( );
	wrapNextLine = false;

	if ( curx < sCurLineLength ) {
		ret = rCursor->getX() - rCurrentLeft < mColumnsVis;
		lastChar = sCurLine[ curx ];
		if ( lastChar != tabChar ) {
			if ( drawMode ) charSelected = selectionPool->isSelected( sCursor );
			rColLength = 1;
		} else {
			lastChar = ' ';
			if ( rCursor->getX( ) == mCurrentLeft ) 
				rColLength = ( rSpaceFill ? rSpaceFill : tabwidth );
			else {
				unsigned int col = rCursor->getX() % tabwidth;
				if ( mCurrentLeft == 0 && rSpaceFill ) {
					if ( col < rSpaceFill ) col = tabwidth + col - rSpaceFill;
					else col -= rSpaceFill;
				}
				rColLength = tabwidth - col;
			}
		}
		sColLength = 1;

		rCursor->setX( rCursor->getX() + rColLength );
		sCursor->setX( sCursor->getX() + sColLength );
		if ( drawMode && ret ) rHLa += sColLength;
	}
	unsigned int shift = ( ! drawMode && ( YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT == mMode && sCurLineLength > 0 ) ) ? 1 : 0;
	wrapNextLine = ( wrap && rCursor->getX() >= mColumnsVis && curx < sCurLineLength + shift );
	sLineLength = wrapNextLine ? 0 : 1;

	return ret;
}

const QChar& YZView::drawChar( ) {
	return lastChar;
}
unsigned int YZView::drawLength( ) {
	return rColLength;
}
unsigned int YZView::drawHeight ( ) {
	return rLineLength;
}
unsigned int YZView::lineHeight ( ) {
	return sLineLength;
}
bool YZView::drawSelected( ) {
	return charSelected;
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
	return sCursor->getY( ) + 1;
}

unsigned int YZView::drawTotalHeight() {

	unsigned int totalHeight = 0;

	initDraw( 0, 0, 0, 0 );
	drawMode = false;
	while( sCursor->getY() < mBuffer->lineCount() ) {
		drawNextLine();
		totalHeight += drawHeight();
	}

	return totalHeight;
}
	

void YZView::substitute(const QString& range, const QString& search, const QString& replace, const QString& option) {
	yzDebug() << "substitute : " << range << ":" << search << ":" << replace << ":" << option << endl;
	//TODO : better range support, better options support
	unsigned int startLine = mCursor->getY();
	unsigned int endLine = mCursor->getY();
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
	stickyCol = dCursor->getX( );
	mBuffer->mergeNextLine( line );
	paintEvent( dCurrentLeft, dCursor->getY(), mColumnsVis, mLinesVis - ( dCursor->getY() - dCurrentTop ) );
}

QString YZView::joinLine ( const QString& , YZCommandArgs ) {
	//reset the input buffer
	purgeInputBuffer();
	joinLine( mCursor->getY() );
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

