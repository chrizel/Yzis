/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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
#include "view.h"
#include "debug.h"
#include <qkeysequence.h>

YZView::YZView(YZBuffer *_b, YZSession *sess, int lines) {
	myId = YZSession::mNbViews++;
	yzDebug() << "New View created with UID : " << myId << endl;
	mSession = sess;
	mBuffer	= _b;
	mLinesVis = lines;
	mCursor = new YZCursor(this);
	mMaxX = 0;
	mMode = YZ_VIEW_MODE_COMMAND;
	mCurrentTop = mCursor->getY();
	QString line = mBuffer->data(mCurrentTop);
	if (!line.isNull()) mMaxX = line.length()-1;

	mBuffer->addView(this);
	mCurrentExItem = 0;
	mExHistory.resize(200);
}

YZView::~YZView() {
	mBuffer->rmView(this); //make my buffer forget about me
}

void YZView::setVisibleLines(int nb) {
	redrawScreen();
	mLinesVis = nb;
}

/* Used by the buffer to post events */
void YZView::sendKey( int c, int modifiers) {
	//ignore some keys
	if ( c == Qt::Key_Shift || c == Qt::Key_Alt || c == Qt::Key_Meta ||c == Qt::Key_Control || c == Qt::Key_CapsLock ) return;

	//map other keys //BAD XXX
	if ( c == Qt::Key_Insert ) c = Qt::Key_I;
	
	QString lin;
	QString key = QChar( tolower( c ) );// = QKeySequence( c );
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
					mBuffer->addNewLine(mCursor->getX(),mCursor->getY());
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
					mBuffer->addChar(mCursor->getX(),mCursor->getY(),"\t");
					gotoxy(mCursor->getX()+1, mCursor->getY() );
					return;
				case Qt::Key_Backspace:
					if (mCursor->getX() == 0) return;
					mBuffer->delChar(mCursor->getX()-1,mCursor->getY(),1);
					gotoxy(mCursor->getX()-1, mCursor->getY() );
					return;
				case Qt::Key_Delete:
					mBuffer->delChar(mCursor->getX(),mCursor->getY(),1);
					return;
				default:
					mBuffer->addChar(mCursor->getX(),mCursor->getY(),key);
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
					mBuffer->addNewLine(mCursor->getX(),mCursor->getY());
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
				default:
					mBuffer->chgChar(mCursor->getX(),mCursor->getY(),key);
					gotoxy(mCursor->getX()+1, mCursor->getY() );
					return;
			}
			break;

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
					setFocusMainWindow();
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
					setFocusMainWindow();
					gotoCommandMode();
					return;
				case Qt::Key_Tab:
					//ignore for now
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
			break;

		case YZ_VIEW_MODE_COMMAND:
			switch ( c ) {
				case Qt::Key_Escape:
					purgeInputBuffer();
					return;
				case Qt::Key_Down:
					key='j';//moveDown( );
					break;//return;
				case Qt::Key_Left:
					key='h';//moveLeft( );
					break;//return;
				case Qt::Key_Right:
					key='l';//moveRight( );
					break;//return;
				case Qt::Key_Up:
					key='k';//moveUp( );
					break;//return;
				case Qt::Key_Delete:
					mBuffer->delChar(mCursor->getX(),mCursor->getY(),1);
					return;
				default:
					break;
			}
			mPreviousChars+=key;
			yzDebug() << "Previous chars : " << mPreviousChars << endl;
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
	static unsigned int lasty = 0; // small speed optimisation
	static QString percentage("All");
	unsigned int y = mCursor->getY();

	if ( y != lasty ) {
		unsigned int nblines = mBuffer->lineCount();
		percentage = QString("%1%").arg( ( unsigned int )( y*100/ ( nblines==0 ? 1 : nblines )));
		if ( mCurrentTop < 1 )  percentage="Top";
		if ( mCurrentTop+mLinesVis >= nblines )  percentage="Bot";
		if ( (mCurrentTop<1 ) &&  ( mCurrentTop+mLinesVis >= nblines ) ) percentage="All";
		lasty=y;
	}

	// FIXME handles tabs here or somwhere else..
	mSession->postEvent(YZEvent::mkEventCursor(myId, y, mCursor->getX(), mCursor->getX(), percentage));
}

void YZView::centerView(unsigned int line) {
	//update current
	int newcurrent = line - mLinesVis / 2;

	if ( newcurrent > ( int( mBuffer->lineCount() ) - int( mLinesVis ) ) )
		newcurrent = mBuffer->lineCount() - mLinesVis;
	if ( newcurrent < 0 ) newcurrent = 0;

	if ( newcurrent== int( mCurrentTop ) ) return;

	//redraw the screen
	mCurrentTop = newcurrent;
	redrawScreen();
}

void YZView::redrawScreen() {
	yzDebug() << "View " << myId << " redraw" << endl;
	mSession->postEvent(YZEvent::mkEventRedraw(myId) );
	updateCursor();
}

/*
 * all the goto-like commands
 */

void YZView::gotoxy(unsigned int nextx, unsigned int nexty) {
	QString lin;

	// check positions
	if ( ( int )nexty < 0 ) nexty = 0;
	else if ( nexty >=  mBuffer->lineCount() ) nexty = mBuffer->lineCount() - 1;
	mCursor->setY( nexty );

	lin = mBuffer->data(nexty);
	if ( !lin.isNull() ) mMaxX = (lin.length() == 0) ? 0 : lin.length()-1; 
	if ( YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT==mMode ) {
		/* in edit mode, at end of line, cursor can be on +1 */
		if ( nextx > mMaxX+1 ) nextx = mMaxX+1;
	} else {
		if ( nextx > mMaxX ) nextx = mMaxX;
	}

	if ( ( int )nextx < 0 ) nextx = 0;
	mCursor->setX( nextx );

	//make sure this line is visible
	if ( ! isLineVisible( nexty ) ) centerView( nexty );

	/* do it */
	updateCursor();

}

QString YZView::moveDown( const QString& inputsBuff ) {
	int nb_lines=1;//default : one line down

	//check the arguments
	if ( !inputsBuff.isNull() ) {
		int i=0;
		while ( inputsBuff[i].isDigit() )
			i++; //go on
		bool test;
		nb_lines = inputsBuff.left( i ).toInt( &test );
		if ( !test ) nb_lines=1;
	}

	//execute the code
	gotoxy(mCursor->getX(), mCursor->getY() + nb_lines);

	//reset the input buffer
	purgeInputBuffer();

	//return something
	return QString::null;
}

QString YZView::moveUp( const QString& inputsBuff ) {
	int nb_lines=1;//default : one line up

	//check the arguments
	if ( !inputsBuff.isNull() ) {
		int i=0;
		while ( inputsBuff[i].isDigit() )
			i++; //go on
		bool test;
		nb_lines = inputsBuff.left( i ).toInt( &test );
		if ( !test ) nb_lines=1;
	}

	//execute the code
	gotoxy(mCursor->getX(), mCursor->getY() ? mCursor->getY() - nb_lines : 0 );

	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveLeft( const QString& inputsBuff ) {
	int nb_cols=1;//default : one line left

	//check the arguments
	if ( !inputsBuff.isNull() ) {
		int i=0;
		while ( inputsBuff[i].isDigit() )
			i++; //go on
		bool test;
		nb_cols = inputsBuff.left( i ).toInt( &test );
		if ( !test ) nb_cols=1;
	}

	//execute the code
	gotoxy( mCursor->getX() ? mCursor->getX() - nb_cols : 0 , mCursor->getY());

	//reset the input buffer
	purgeInputBuffer();

	//return something
	return QString::null;
}

QString YZView::moveRight( const QString& inputsBuff ) {
	int nb_cols=1;//default : one column right
	
	//check the arguments
	if ( !inputsBuff.isNull() ) {
		int i=0;
		while ( inputsBuff[i].isDigit() )
			i++; //go on
		bool test;
		nb_cols = inputsBuff.left( i ).toInt( &test );
		if ( !test ) nb_cols=1;
	}

	//execute the code
	gotoxy(mCursor->getX() + nb_cols , mCursor->getY());
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveToFirstNonBlankOfLine( const QString& ) {
	//execute the code
	gotoxy(mBuffer->firstNonBlankChar(mCursor->getY()) , mCursor->getY());
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::moveToStartOfLine( const QString& ) {
	//execute the code
	gotoxy(0 , mCursor->getY());
	
	//reset the input buffer
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::gotoLine(const QString& inputsBuff) {
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


QString YZView::moveToEndOfLine( const QString& ) {
	QString lin;
	
	gotoxy( mMaxX+10 , mCursor->getY());
	
	purgeInputBuffer();
	//return something
	return QString::null;
}

QString YZView::deleteCharacter( const QString& inputsBuff ) {
	QString lin;
	int nb_cols=1;//default : one row right
	
	//check the arguments
	if ( !inputsBuff.isNull() ) {
		int i=0;
		while ( inputsBuff[i].isDigit() )
			i++; //go on
		bool test;
		nb_cols = inputsBuff.left( i ).toInt( &test );
		if ( !test ) nb_cols=1;
	}
	
	//execute the code
	mBuffer->delChar(mCursor->getX(), mCursor->getY(), nb_cols);

	//reset the input buffer
	purgeInputBuffer();
	gotoxy( mCursor->getX(), mCursor->getY());

	return QString::null;
}

QString YZView::deleteLine ( const QString& inputsBuff ) {
	int nb_lines=1;//default : one line
	QChar reg='\"';

	QRegExp ex( "(\".)?([0-9]*)(d.|D)" );
	ex.exactMatch( inputsBuff );
	QStringList ls = ex.capturedTexts();
	yzDebug() << "view::deleteLine " << ls << endl;
	
	if ( ls[ 1 ] != "" ) //got a register name ?
		reg = ls[ 2 ][ 1 ];
	if ( ls[ 2 ] != "" )  //number of lines ?
		nb_lines = ls[ 2 ].toInt();
	
	QStringList buff; //to copy old lines into the register "
	if ( ls[ 3 ] == "dd" ) { //delete whole lines
		for ( int i=0; i<nb_lines; ++i ) {
			buff << mBuffer->data( mCursor->getY() );
			mBuffer->deleteLine( mCursor->getY() );
		}
	} else if ( ls[ 3 ] == "D" || ( ls[ 3 ] == "d$" ) ) { //delete to end of lines
		QString b = mBuffer->data( mCursor->getY() );
		buff << b;
		mBuffer->replaceLine( mCursor->getY(), b.left( mCursor->getX() ) );
		gotoxy( mCursor->getX() - 1, mCursor->getY() );
	}
	YZSession::mRegisters.setRegister( '\"', buff );

	//reset the input buffer
	purgeInputBuffer();

	// prevent bug when deleting the last line
	gotoxy( mCursor->getX(), mCursor->getY());

	return QString::null;
}

QString YZView::openNewLineBefore ( const QString& ) {
	mBuffer->addNewLine(0,mCursor->getY());
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();

	gotoxy(0,mCursor->getY());

	//reset the input buffer
	purgeInputBuffer();
	return QString::null;
}

QString YZView::openNewLineAfter ( const QString& ) {
	mBuffer->addNewLine(0,mCursor->getY()+1);
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();

	gotoxy( 0,mCursor->getY()+1 );

	return QString::null;
}

QString YZView::append ( const QString& ) {
	//reset the input buffer
	purgeInputBuffer();
	gotoInsertMode();
	gotoxy(mCursor->getX()+1, mCursor->getY() );

	return QString::null;
}

QString YZView::appendAtEOL ( const QString& ) {
	//reset the input buffer
	purgeInputBuffer();
	moveToEndOfLine();
	append();

	return QString::null;
}

QString YZView::gotoCommandMode( ) {
	mMode = YZ_VIEW_MODE_COMMAND;
	purgeInputBuffer();
	mSession->postEvent(YZEvent::mkEventStatus(myId,"Command mode"));
	return QString::null;
}

QString YZView::gotoExMode(const QString&) {
	mMode = YZ_VIEW_MODE_EX;
	mSession->postEvent(YZEvent::mkEventStatus(myId,"-- EX --"));
	setFocusCommandLine();
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoInsertMode(const QString&) {
	mMode = YZ_VIEW_MODE_INSERT;
	mSession->postEvent(YZEvent::mkEventStatus(myId,"-- INSERT --"));
	purgeInputBuffer();
	return QString::null;
}

QString YZView::gotoReplaceMode(const QString&) {
	mMode = YZ_VIEW_MODE_REPLACE;
	mSession->postEvent(YZEvent::mkEventStatus(myId,"-- REPLACE --") );
	purgeInputBuffer();
	return QString::null;
}

QString YZView::copy( const QString& inputsBuff ) {
	//default register to use
	QChar reg = '"';
	int nb_lines = 1;
	QRegExp ex ( "(\".)?([0-9]*)(y.|Y)" );
	ex.exactMatch( inputsBuff );
	QStringList ls = ex.capturedTexts();
	yzDebug() << "view::copy " << ls << endl;
	
	if ( ls[ 1 ] != "" ) //got a register name ?
		reg = ls[ 2 ][ 1 ];
	if ( ls[ 2 ] != "" )  //number of lines ?
		nb_lines = ls[ 2 ].toInt();
	
	QStringList list;
	if ( ls[ 3 ] == "yy" ) {
		list << QString::null; //just a marker
		for (int i = 0 ; i < nb_lines; i++ )
			list << mBuffer->data(mCursor->getY()+i);
	} else if ( ls[ 3 ] == "Y" ) {
		list << QString::null; //just a marker
		for (int i = 0 ; i < nb_lines; i++ )
			list << mBuffer->data(mCursor->getY()+i);
	} else if ( ls[ 3 ] == "y$" ) {
		QString lin = mBuffer->data( mCursor->getY() );
		list << lin.mid(mCursor->getX());
	}
	YZSession::mRegisters.setRegister( reg, list );

	purgeInputBuffer();
	return QString::null;
}

QString YZView::paste( const QString& inputsBuff ) {
	//default register to use
	QRegExp ex("(\".)?(p|P)");
	ex.exactMatch( inputsBuff );
	QStringList ls = ex.capturedTexts();
	yzDebug() << "view::paste " << ls << endl;

	QChar reg = '"';
	if ( ls[ 1 ] != "" ) //got a register name ?
		reg = ls[ 1 ][ 1 ];

	QStringList list = YZSession::mRegisters.getRegister( reg );
	//save cursor pos
	int curx = mCursor->getX();
	int cury = mCursor->getY();

	uint i = 0;
	if ( ls[ 2 ] == "p" ) { //paste after current char
		QString nl = mBuffer->data( mCursor->getY() );
		if ( !list[ 0 ].isNull() ) {
			yzDebug() << "First line not NULL !" << endl;
			nl = nl.left( mCursor->getX()+1 ) + list[ 0 ] + nl.mid( mCursor->getX()+1 );
			mBuffer->replaceLine(mCursor->getY(), nl);
		}
		i++;
		while ( i < list.size() ) {
			mBuffer->insertLine(list[ i++ ], mCursor->getY()+1);
			gotoxy( mCursor->getX(), mCursor->getY()+1 );
		}
	} else if ( ls[ 2 ] == "P" ) { //paste before current char
		if ( list[ 0 ].isNull() )
			i++;
		while ( i < ( list[ 0 ].isNull() ? list.size() : list.size() - 1 ) ) {
			mBuffer->insertLine(list[ i++ ], mCursor->getY());
			gotoxy( mCursor->getX(), mCursor->getY()+1 );
		}
		if ( !list[ 0 ].isNull() ) {
			QString nl = mBuffer->data( mCursor->getY() );
			nl = nl.left( mCursor->getX() ) + list[ list.size()-1 ] + nl.mid( mCursor->getX() );
			mBuffer->replaceLine(mCursor->getY(), nl);
		}
	}
	//restore cursor pos
	gotoxy( curx, cury );

	purgeInputBuffer();
	return QString::null;
}

