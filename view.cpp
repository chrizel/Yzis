/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <marchand@kde.org>
 *  Thomas Capricelli <orzel@freehackers.org>
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
#include "view.h"
#include "debug.h"
#include "undo.h"

static const int tabLength( 8 );
static const QChar tabChar( '\t' );

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
	viewInformation.l1 = viewInformation.l2 = viewInformation.c1 = viewInformation.c2 = 0;
	viewInformation.percentage = "";
}

YZView::~YZView() {
	mBuffer->rmView(this); //make my buffer forget about me
}

void YZView::setVisibleArea(int c, int l) {
	yzDebug() << "setVisibleArea : " << c << " " << l << endl;
	mLinesVis = l;
	mColumnsVis = c;
	redrawScreen();
}

/* Used by the buffer to post events */
void YZView::sendKey( int c, int modifiers) {
	if ( mBuffer->introShown() ) {
		mBuffer->clearIntro();
		gotoxy( 0,0 );
	}

	//ignore some keys
	if ( c == Qt::Key_Shift || c == Qt::Key_Alt || c == Qt::Key_Meta ||c == Qt::Key_Control || c == Qt::Key_CapsLock ) {
		yzError( )<< "receiving modifiers in c, shouldn't happen" <<endl;
		return;
	}

#if 0
	// useful stuff, but dont commit with  #if 1 :)
	yzDebug()<< "YZView::sendKey : receiving " << 
		( ( modifiers & Qt::ControlButton )?"CONTROL+":"" ) <<
		( ( modifiers & Qt::AltButton )?"ALT+":"" ) <<
		( ( modifiers & Qt::ShiftButton )?"SHIFT+":"" );
	if ( isprint( c ) ) yzDebug() << QString(QChar(c)) << endl;
	else yzDebug()<< "(int) " << c << endl;
#endif

	// handle CONTROL SEQUENCE
	// we copy vim behaviour ^L go through in INSERT/REPLACE/SEARCH
	if ( modifiers & Qt::ControlButton )
	switch(mMode) {
		case YZ_VIEW_MODE_INSERT:
		case YZ_VIEW_MODE_REPLACE:
		case YZ_VIEW_MODE_SEARCH:
			break;// continue
		case YZ_VIEW_MODE_EX:
		case YZ_VIEW_MODE_COMMAND:
			switch ( tolower(c) ) {
				case 'l':
					redrawScreen();
					return;
				default:
					yzWarning()<< "Unhandled control sequence " << c <<endl;
					return;

			}
	};


	// we did for control, and will do for shift, everything else is discarded
	if ( modifiers & ~(Qt::ShiftButton|Qt::ControlButton ) ) {
		// anything else than ShiftButton ?
		yzWarning( )<< "Other modifier than control/shift -> still unhandled" <<endl;
		return;
	}


	//map other keys //BAD XXX
	if ( c == Qt::Key_Insert ) c = Qt::Key_I;
	


	QString lin;
	QString key = QChar( tolower(c) );// = QKeySequence( c );
	//default is lower case unless some modifiers
	if ( modifiers & Qt::ShiftButton )
		key = key.upper();

	switch(mMode) {

		case YZ_VIEW_MODE_INSERT:
			switch ( c ) {
				case Qt::Key_Escape:
					if (mCursor->getX() > mMaxX) {
						gotoxy(mMaxX, mCursor->getY());
					}
					gotoCommandMode( );
					return;
				case Qt::Key_Return:
					mBuffer->insertNewLine(mCursor->getX(),mCursor->getY());
					gotoxy(0, mCursor->getY()+1 );
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
				case Qt::Key_Tab:
					mBuffer->insertChar(mCursor->getX(),mCursor->getY(),"\t");
					gotoxy(mCursor->getX()+1, mCursor->getY() );
					return;
				case Qt::Key_Backspace:
					if (mCursor->getX() == 0 && mCursor->getY() > 0 && 1 /* option_go_back_to_previous_line */) {
						int lineLen = mBuffer->textline( mCursor->getY()-1 ).length();
						mBuffer->mergeNextLine( mCursor->getY() -1 );
						gotoxy(lineLen, mCursor->getY()-1);
						return;
					}
					mBuffer->delChar(mCursor->getX()-1,mCursor->getY(),1);
					gotoxy(mCursor->getX()-1, mCursor->getY() );
					return;
				case Qt::Key_Delete:
					mBuffer->delChar(mCursor->getX(),mCursor->getY(),1);
					return;
				case Qt::Key_PageDown:
					gotoxy(mCursor->getX(), mCursor->getY() + mLinesVis );
					purgeInputBuffer();
					return;
				case Qt::Key_PageUp:
					gotoxy(mCursor->getX(), mCursor->getY() - mLinesVis );
					purgeInputBuffer();
					return;
				default:
					mBuffer->insertChar(mCursor->getX(),mCursor->getY(),key);
					gotoxy(mCursor->getX()+1, mCursor->getY() );
					return;
			}
			break;

		case YZ_VIEW_MODE_REPLACE:
			switch ( c ) {
				case Qt::Key_Escape:
					if (mCursor->getX() > mMaxX) {
						gotoxy(mMaxX, mCursor->getY());
					}
					gotoCommandMode( );
					return;
				case Qt::Key_Return:
					mBuffer->insertNewLine(mCursor->getX(),mCursor->getY());
					gotoxy(0, mCursor->getY()+1 );
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
				case Qt::Key_Tab:
					mBuffer->chgChar(mCursor->getX(),mCursor->getY(),"\t");
					gotoxy(mCursor->getX()+1, mCursor->getY() );
					return;
				case Qt::Key_Backspace:
					if (mCursor->getX() == 0) return;
					gotoxy(mCursor->getX()-1, mCursor->getY() );
					return;
				case Qt::Key_PageDown:
					gotoxy(mCursor->getX(), mCursor->getY() + mLinesVis );
					purgeInputBuffer();
					return;
				case Qt::Key_PageUp:
					gotoxy(mCursor->getX(), mCursor->getY() - mLinesVis );
					purgeInputBuffer();
					return;
				default:
					mBuffer->chgChar(mCursor->getX(),mCursor->getY(),key);
					gotoxy(mCursor->getX()+1, mCursor->getY() );
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
					gotoCommandMode();
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
					gotoCommandMode();
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
					gotoCommandMode();
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
					gotoCommandMode();
					return;
				case Qt::Key_Tab:
					//ignore for now
					return;
				case Qt::Key_Backspace:
				{
					QString back = getCommandLineText();
					if ( back.isEmpty() ) {
						mSession->setFocusMainWindow();
						gotoCommandMode( );
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

		case YZ_VIEW_MODE_COMMAND:
			switch ( c ) {
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
					mBuffer->delChar(mCursor->getX(),mCursor->getY(),1);
					return;
				case Qt::Key_PageDown:
					gotoxy(mCursor->getX(), mCursor->getY() + mLinesVis );
					purgeInputBuffer();
					return;
				case Qt::Key_PageUp:
					gotoxy(mCursor->getX(), mCursor->getY() - mLinesVis );
					purgeInputBuffer();
					return;
				default:
					break;
			}
			mPreviousChars+=key;
//			yzDebug() << "Previous chars : (" << int( (mPreviousChars.latin1())[0] )<< ") " << mPreviousChars << endl;
			if ( mSession ) {
				int error = 0;
				mSession->getPool()->execCommand(this, mPreviousChars, &error);
				if ( error == 1 ) purgeInputBuffer(); // no matching command
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

	viewInformation.l1 = viewInformation.l2 = y; //XXX wrapped lines
	viewInformation.c1 = mCursor->getX(); 
	viewInformation.c2 = dCursor->getX(); 

	syncViewInfo();
}

void YZView::centerViewHorizontally(unsigned int column) {
	yzDebug() << "centerViewHorizontally " << column << endl;
	unsigned int newcurrentLeft = column - mColumnsVis / 2;

	if ( newcurrentLeft > ( mMaxX - mColumnsVis ) ) {
		if ( column > mColumnsVis/2 )
			newcurrentLeft = mMaxX - mColumnsVis;
		else newcurrentLeft = 0;
	}
	if ( newcurrentLeft == mCurrentLeft ) return;

	if (newcurrentLeft > 0) {
		initDraw( 0, 0, 0, 0 );
		while( sCursor->getY( ) < mCursor->getY( ))
			drawNextLine( );
		while ( rCursor->getX( ) < newcurrentLeft ) {
			drawNextCol( );
			drawChar( );
		}
		dCurrentLeft = rCursor->getX( ) + dCurrentLeft;
		mCurrentLeft = sCursor->getX( ) + mCurrentLeft;

		initDraw ( );
	} else {
		dCurrentLeft = 0;
		mCurrentLeft = 0;
	}
	yzDebug() << "centerViewHorizontally d:" << dCurrentLeft << ", m:" << mCurrentLeft << ", fill:" << rSpaceFill << endl;
	
	redrawScreen();
}

void YZView::centerViewVertically(unsigned int line) {
	unsigned int newcurrent = line - mLinesVis / 2;

	if ( newcurrent > ( mBuffer->lineCount() - mLinesVis ) ) {
		if ( line > mLinesVis/2 )
			newcurrent = mBuffer->lineCount() - mLinesVis;
		else newcurrent = 0;
	}
	if ( newcurrent == mCurrentTop ) return;
	
	//redraw the screen
	mCurrentTop = newcurrent > 0 ? newcurrent : 0;
	dCurrentTop = newcurrent > 0 ? newcurrent : 0;

//	yzDebug() << "centerVertically: m:" << mCurrentTop << "d: " << dCurrentTop << endl;

	redrawScreen();
}

//drop me ? (==move it to GUIs directly)
void YZView::redrawScreen() {
	yzDebug() << "View " << myId << " redraw" << endl;
	refreshScreen( );
	updateCursor();
}

/*
 * all the goto-like commands
 */

void YZView::gotoxy(unsigned int nextx, unsigned int nexty) {
	QString lin;
	if ( mBuffer->introShown() ) mBuffer->clearIntro();

	// check positions
	if ( ( int )nexty < 0 ) nexty = 0;
	else if ( nexty >=  mBuffer->lineCount() ) nexty = mBuffer->lineCount() - 1;
	mCursor->setY( nexty );

	lin = mBuffer->textline(nexty);
	if ( !lin.isNull() ) mMaxX = (lin.length() == 0) ? 0 : lin.length()-1; 
	if ( YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT==mMode && lin.length() > 0 ) {
		/* in edit mode, at end of line, cursor can be on +1 */
		if ( nextx > mMaxX+1 ) nextx = mMaxX+1;
	} else {
		if ( nextx > mMaxX ) nextx = mMaxX;
	}

	if ( ( int )nextx < 0 ) nextx = 0;
	mCursor->setX( nextx );

	initDraw(0, 0, 0, 0);
	while ( sCursor->getY() < mCursor->getY() ) drawNextLine( );
	while ( sCursor->getX() < mCursor->getX() ) {
		drawNextCol( );
		drawChar( );
	}

	dCursor->setX( rCursor->getX() );
	dCursor->setY( rCursor->getY() );

//	yzDebug( ) << "mCursor:" << mCursor->getX( ) << "," << mCursor->getY( ) << endl;
//	yzDebug( ) << "dCursor:" << dCursor->getX( ) << "," << dCursor->getY( ) << endl;

	
	//make sure this line is visible
	if ( !isLineVisible( dCursor->getY() ) ) centerViewVertically( dCursor->getY( ) );
	if ( !isColumnVisible( dCursor->getX(), dCursor->getY() ) ) centerViewHorizontally( dCursor->getX( ) );

//	yzDebug( ) << "(2) mCursor:" << mCursor->getX( ) << "," << mCursor->getY( ) << endl;
//	yzDebug( ) << "(2) dCursor:" << dCursor->getX( ) << "," << dCursor->getY( ) << endl;

	/* do it */
	updateCursor();
}

QString YZView::moveDown( const QString& , YZCommandArgs args ) {
	int nb_lines=args.count;

	//execute the code
	gotoxy(mCursor->getX(), mCursor->getY() + nb_lines);

	//reset the input buffer
	purgeInputBuffer();

	//return something
	return QString::null;
}

QString YZView::moveUp( const QString& , YZCommandArgs args ) {
	int nb_lines=args.count;

	//execute the code
	gotoxy(mCursor->getX(), mCursor->getY() ? mCursor->getY() - nb_lines : 0 );

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

	//reset the input buffer
	purgeInputBuffer();

	//return something
	return QString::null;
}

QString YZView::moveRight( const QString& , YZCommandArgs args ) {
	int nb_cols=args.count;
	
	//execute the code
	gotoxy(mCursor->getX() + nb_cols , mCursor->getY());
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveToFirstNonBlankOfLine( const QString&, YZCommandArgs ) {
	//execute the code
	gotoxy(mBuffer->firstNonBlankChar(mCursor->getY()) , mCursor->getY());
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveToStartOfLine( const QString&, YZCommandArgs ) {
	//execute the code
	gotoxy(0 , mCursor->getY());
	
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

	/* if line is null, we do not want to go to line -1,
	 * this can happen if the file is empty for exemple */
	if ( !line ) line++;
	if (line > mBuffer->lineCount()) line = mBuffer->lineCount();

	if (/* XXX configuration startofline */ 1 ) {
		gotoxy(mBuffer->firstNonBlankChar(line-1), line-1);
	} else {
		gotoxy(mCursor->getY(), line-1);
	}

	purgeInputBuffer();
	return QString::null; //return something
}

// end of goto-like command


QString YZView::moveToEndOfLine( const QString&, YZCommandArgs ) {
	gotoxy( mMaxX+10 , mCursor->getY());
	
	purgeInputBuffer();
	return QString::null;
}

QString YZView::deleteCharacter( const QString& , YZCommandArgs args ) {
	int nb_cols=args.count;
	
	//execute the code
	mBuffer->delChar(mCursor->getX(), mCursor->getY(), nb_cols);

	//reset the input buffer
	purgeInputBuffer();
	gotoxy( mCursor->getX(), mCursor->getY());

	return QString::null;
}

QString YZView::deleteLine ( const QString& /*inputsBuff*/, YZCommandArgs args ) {
	int nb_lines=args.count;
	QChar reg=args.registr;

	QStringList buff; //to copy old lines into the register "
	if ( args.command == "dd" ) { //delete whole lines
		for ( int i=0; i<nb_lines; ++i ) {
			buff << mBuffer->textline( mCursor->getY() );
			mBuffer->deleteLine( mCursor->getY() );
		}
	} else if ( args.command == "D" || args.command == "d$"  ) { //delete to end of lines
		QString b = mBuffer->textline( mCursor->getY() );
		buff << b;
		mBuffer->replaceLine( b.left( mCursor->getX() ), mCursor->getY());
		gotoxy( mCursor->getX() - 1, mCursor->getY() );
	}
	YZSession::mRegisters.setRegister( reg, buff );

	//reset the input buffer
	purgeInputBuffer();

	// prevent bug when deleting the last line
	gotoxy( mCursor->getX(), mCursor->getY());

	return QString::null;
}

QString YZView::openNewLineBefore ( const QString&, YZCommandArgs ) {
	mBuffer->insertNewLine(0,mCursor->getY());
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();

	gotoxy(0,mCursor->getY());

	//reset the input buffer
	purgeInputBuffer();
	return QString::null;
}

QString YZView::openNewLineAfter ( const QString&, YZCommandArgs ) {
	mBuffer->insertNewLine(0,mCursor->getY()+1);
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();

	gotoxy( 0,mCursor->getY()+1 );

	return QString::null;
}

QString YZView::append ( const QString&, YZCommandArgs ) {
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();
	gotoxy(mCursor->getX()+1, mCursor->getY() );

	return QString::null;
}

QString YZView::appendAtEOL ( const QString&, YZCommandArgs ) {
	//reset the input buffer
	purgeInputBuffer();
	moveToEndOfLine();
	append();

	return QString::null;
}

QString YZView::gotoCommandMode( ) {
	mBuffer->undoBuffer()->commitUndoItem();
	mMode = YZ_VIEW_MODE_COMMAND;
	modeChanged();
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoExMode(const QString&, YZCommandArgs ) {
	mMode = YZ_VIEW_MODE_EX;
	modeChanged();
	mSession->setFocusCommandLine();
	purgeInputBuffer();
	setCommandLineText( "" );
	return QString::null;
}

QString YZView::gotoInsertMode(const QString&, YZCommandArgs ) {
	mBuffer->undoBuffer()->commitUndoItem();
	mMode = YZ_VIEW_MODE_INSERT;
	modeChanged();
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoReplaceMode(const QString&, YZCommandArgs ) {
	mBuffer->undoBuffer()->commitUndoItem();
	mMode = YZ_VIEW_MODE_REPLACE;
	modeChanged();
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoSearchMode( const QString& inputsBuff, YZCommandArgs /*args */) {
	reverseSearch = (inputsBuff[ 0 ] == '?' );
	mMode = YZ_VIEW_MODE_SEARCH;
	modeChanged();
	purgeInputBuffer();
	setCommandLineText( "" );
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
	} else if ( args.command == "Y" ) {
		list << QString::null; //just a marker
		for (int i = 0 ; i < nb_lines; i++ )
			list << mBuffer->textline(mCursor->getY()+i);
	} else if ( args.command == "y$" ) {
		QString lin = mBuffer->textline( mCursor->getY() );
		list << lin.mid(mCursor->getX());
	}
	YZSession::mRegisters.setRegister( args.registr, list );

	purgeInputBuffer();
	return QString::null;
}

QString YZView::paste( const QString& , YZCommandArgs args ) {
	QStringList list = YZSession::mRegisters.getRegister( args.registr );
	//save cursor pos
	int curx = mCursor->getX();
	int cury = mCursor->getY();

	uint i = 0;
	if ( args.command == "p" ) { //paste after current char
		QString nl = mBuffer->textline( mCursor->getY() );
		if ( !list[ 0 ].isNull() ) {
			yzDebug() << "First line not NULL !" << endl;
			nl = nl.left( mCursor->getX()+1 ) + list[ 0 ] + nl.mid( mCursor->getX()+1 );
			mBuffer->replaceLine(nl, mCursor->getY());
		}
		i++;
		while ( i < list.size() ) {
			mBuffer->insertLine(list[ i++ ], mCursor->getY()+1);
			gotoxy( mCursor->getX(), mCursor->getY()+1 );
		}
	} else if ( args.command == "P" ) { //paste before current char
		if ( list[ 0 ].isNull() )
			i++;
		while ( i < ( list[ 0 ].isNull() ? list.size() : list.size() - 1 ) ) {
			mBuffer->insertLine(list[ i++ ], mCursor->getY());
			gotoxy( mCursor->getX(), mCursor->getY()+1 );
		}
		if ( !list[ 0 ].isNull() ) {
			QString nl = mBuffer->textline( mCursor->getY() );
			nl = nl.left( mCursor->getX() ) + list[ list.size()-1 ] + nl.mid( mCursor->getX() );
			mBuffer->replaceLine(nl, mCursor->getY());
		}
	}
	//restore cursor pos
	gotoxy( curx, cury );

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
			gotoxy( currentMatchColumn, currentMatchLine );
			return true;
		} else {
			yzDebug() << "No match on this line" << endl;
			if ( reverseSearch )
				currentMatchColumn=-1;
			else
				currentMatchColumn=0; //reset the column (only valid for the first line we check)
		}
	}
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

void YZView::initDraw( ) {
	initDraw( mCurrentLeft, mCurrentTop, dCurrentLeft, dCurrentTop );
}

void YZView::initDraw( unsigned int sLeft, unsigned int sTop, 
			unsigned int rLeft, unsigned int rTop) {

	sCurrentLeft = sLeft;
	sCurrentTop = sTop;
	rCurrentLeft = rLeft;
	rCurrentTop = rTop;

//	yzDebug() << "/ initScreenClip sCurrentLeft(" << sCurrentLeft << "), sCurrentTop(" << sCurrentTop << ")" << endl;
	sCursor->setX( sCurrentLeft );
	sCursor->setY( sCurrentTop );

//	yzDebug() << "\\ initScreenClip rCurrentLeft(" << rCurrentLeft << "), rCurrentTop(" << rCurrentTop << ")" << endl;
	rCursor->setX( rCurrentLeft );
	rCursor->setY( rCurrentTop );

	rLineLength = 0;
	rColLength = 0;
	sLineLength = 0;
	sColLength = 0;

}

bool YZView::drawNextLine( ) {

	// update sCursor
	sCursor->setX( sCurrentLeft );
	sCursor->setY( sCursor->getY() + sLineLength );

	// update dCusor
	rCursor->setY( rCursor->getY( ) + rLineLength );

	rLineLength = 1;
	sLineLength = 1;
	sColLength = 0;
	rColLength = 0;
	rSpaceFill = 0;

	if ( sCursor->getY() < mBuffer->lineCount() ) {
		
		YZLine *yl = mBuffer->yzline( sCursor->getY() );
		sCurLine = yl->data();
		rCursor->setX( rCurrentLeft );
		if (rCurrentLeft > 0) {

			sCursor->setX( 0 );
			rCursor->setX( 0 );

			while( rCursor->getX( ) < rCurrentLeft - 1 ) {
				drawNextCol( );
				drawChar( );
			}

			rSpaceFill = (rCurrentLeft % tabLength);

/*			yzDebug() << "Draw next line : spaceFill:" << rSpaceFill << ", rX(current):" << rCurrentLeft 
					<< ", rX(cursor):" << rCursor->getX() 
					<< ", rY:" << rCursor->getY() << ", sX:" << sCursor->getX( ) << ", sY:" << sCursor->getY ()
					<< ", drawLength:" << drawLength() << endl; 
*/
			if (drawLength () > 1) {
				rColLength = 1;
				sCursor->setX (sCursor->getX () + 1);
			}
		}

		if ( ( rCursor->getY() - rCurrentTop ) < mLinesVis ) {
			/* highlight stuff */
			rHLa = NULL;
			if ( yl->length() != 0 ) {
				rHLa = yl->attributes();
			}
			rHLnoAttribs = !rHLa;
			rHLa = rHLa + sCurrentLeft;
			rHLAttributes = 0L;
			YzisHighlighting * highlight = mBuffer->highlight();
			if ( highlight )
				rHLAttributes = highlight->attributes( 0 )->data( );
			rHLAttributesLen = rHLAttributes ? highlight->attributes( 0 )->size() : 0;

			return true;
		}
	} else {
		sCurLine = "";
//		yzDebug() << "drawNextLine out of array (r" << rCursor->getY() << ",s" << sCursor->getY() << "), rCurrentTop=" << rCurrentTop << endl;
	}
	return false;
}

bool YZView::drawNextCol( ) {

	// update dCursor position
	rCursor->setX( rCursor->getX( ) + rColLength );
	// update sCursor position
	sCursor->setX ( sCursor->getX() + sColLength );

	rHLa += sColLength;

	return ( rCursor->getX( ) - rCurrentLeft < mColumnsVis 
		&& sCursor->getX( ) < sCurLine.length() );
}

QChar YZView::drawChar( ) {
	
	QChar ch = ' ';
	unsigned int curx = sCursor->getX( );
	if ( curx < sCurLine.length( ) ) {
		ch = sCurLine[ curx ];
	}

	sColLength = 1;
	rColLength = 1;
	if (rSpaceFill == tabLength) rSpaceFill = 0;
	if ( ch == tabChar ) {
		rColLength = tabLength - rSpaceFill;	
		ch = ' ';
	}
	rSpaceFill += rColLength;

	return ch;
}

int YZView::drawLength( ) {
	return rColLength;
}

int YZView::drawHeight ( ) {
	return rLineLength;
}
const QColor& YZView::drawColor ( ) {
	QColor fake;
	YzisAttribute hl;
	YzisAttribute * curAt = ( !rHLnoAttribs && (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt ) {
		hl += * curAt;
		return hl.textColor();
	}
	return fake;
}

