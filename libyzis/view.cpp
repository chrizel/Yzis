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

#include "config.h"
#include <cstdlib>
#include <ctype.h>
#include <qkeysequence.h>
#include <math.h>
#include "view.h"
#include "viewcursor.h"
#include "debug.h"
#include "undo.h"
#ifdef HAVE_LIBPS
#include "printer.h"
#endif
#include "qtprinter.h"
#include "cursor.h"
#include "internal_options.h"
#include "registers.h"
#include "buffer.h"
#include "swapfile.h"
#include "mark.h"
#include "action.h"
#include "commands.h"
#include "session.h"
#include "linesearch.h"
#include "mapping.h"

#define STICKY_COL_ENDLINE -1

#define GET_STRING_WIDTH( s ) ( isFontFixed ? s.length() : stringWidth( s ) )
#define GET_CHAR_WIDTH( c ) ( isFontFixed ? 1 : charWidth( c ) )

static const QChar tabChar( '\t' );
static QColor fake/*( "white" )*/;
static QColor blue( "blue" );


/**
 * class YZView
 */

YZView::YZView(YZBuffer *_b, YZSession *sess, int lines) {
	myId = YZSession::mNbViews++;
	yzDebug() << "New View created with UID : " << myId << endl;
	YZASSERT( _b ); YZASSERT( sess );
	mSession = sess;
	mBuffer	= _b;
	mLineSearch = new YZLineSearch( this );
	mLinesVis = lines;
	mColumnsVis = 0;

	//order matters here ;)
	mModes << tr("[ Insert ]") << tr("[ Replace ]") <<tr("[ Awaiting Command ]") <<tr("[ Ex ]") <<tr("[ Search ]")
		<< tr("[ Open ]") << tr("[ Introduction ]") << tr("[ Visual ]") << tr("[ Visual Line ]") << tr("Yzis Ready");
	mainCursor = new YZViewCursor( this );
	workCursor = new YZViewCursor( this );

	origPos = new YZCursor( this );

	/* start of visual mode */
	mVisualCursor = new YZCursor(this);
	dVisualCursor = new YZCursor(this);

	beginChanges = new YZCursor( this );

	stickyCol = 0;

	mPrevMode = mMode = YZ_VIEW_MODE_COMMAND;
	mapMode = 0;
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
	mFillChar = ' ';

	lineDY = 0;
	tabstop = getLocalIntOption("tabstop");
	wrap = getLocalBoolOption( "wrap" );

	setPaintAutoCommit();
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
	delete beginChanges;
}

void YZView::setupKeys() {
	// register keys with modifiers
	QPtrList<const YZCommand> commands = mSession->getPool()->commands;
	for ( commands.first(); commands.current(); commands.next() ) {
		const QString& keys = commands.current()->keySeq();
		if ( keys.find( "<CTRL>" ) > -1 || keys.find( "<ALT>" ) > -1 ) {
			yzDebug() << "registerModifierKeys " << keys << endl;
			registerModifierKeys( keys );
		}
	}
}

void YZView::setVisibleArea(int c, int l, bool refresh) {
	yzDebug() << "YZView::setVisibleArea(" << c << "," << l << ");" << endl;
	mLinesVis = l;
	mColumnsVis = c;
	if( refresh ) {
		recalcScreen();
	}
}

void YZView::recalcScreen( ) {
	abortPaintEvent();
	refreshScreen();
	YZCursor buffer = *mainCursor->buffer();
	mainCursor->reset();
	gotoxy( buffer.getX(), buffer.getY() );
}

void YZView::sendMultipleKey(const QString& keys) {
	yzDebug() << "sendMultipleKey " << keys << endl;
	for ( unsigned int i = 0 ; i < keys.length(); i++ ) {
		QString key = keys.mid( i );
		if ( key.startsWith( "<CTRL>" ) ) {
			sendKey (key.mid( 6,1 ), "<CTRL>" );
			i+=6;
			continue;
		} else if ( key.startsWith( "<ALT>" ) ) {
			sendKey (key.mid( 5,1 ), "<ALT>" );
			i+=5;
			continue;
		} else if ( key.startsWith( "<SHIFT>" ) ) {
			sendKey (key.mid( 7,1 ), "<SHIFT>" );
			i+=7;
			continue;
		} else if ( key.startsWith( "<ESC>" ) ) {
			sendKey ( "<ESC>" );
			i+=4;
			continue;
		} else if ( key.startsWith( "<ENTER>" ) ) {
			sendKey ( "<ENTER>" );
			i+=6;
			continue;
		} else if ( key.startsWith( "<TAB>" ) ) {
			sendKey ( "<TAB>" );
			i+=4;
			continue;
		} else if ( key.startsWith( "<UP>" ) ) {
			sendKey ( "<UP>" );
			i+=3;
			continue;
		} else if ( key.startsWith( "<DOWN>" ) ) {
			sendKey ( "<DOWN>" );
			i+=5;
			continue;
		} else if ( key.startsWith( "<RIGHT>" ) ) {
			sendKey ( "<RIGHT>" );
			i+=6;
			continue;
		} else if ( key.startsWith( "<LEFT>" ) ) {
			sendKey ( "<LEFT>" );
			i+=5;
			continue;
		} else
			sendKey( key.mid( 0,1 ) );
	}
}

void YZView::sendKey( const QString& _key, const QString& _modifiers) {
//	yzDebug() << "sendKey : " << _key << " " << _modifiers << endl;

	QString key=_key;
	QString modifiers=_modifiers;
	if ( _key == "<SHIFT>" || _key == "<CTRL>" || _key == "<ALT>" ) return; //we are not supposed to received modifiers in key

	if ( mRegs.count() > 0 ) {
		for ( QValueList<QChar>::iterator it = mRegs.begin(); it != mRegs.end(); it++ ) {
			QStringList list;
		   	list << YZSession::mRegisters.getRegister( *it )[ 0 ] + modifiers + _key;
			YZSession::mRegisters.setRegister( *it, list);
		}
	}

	if ( modifiers.contains ("<SHIFT>")) {//usefull ?
		key = key.upper();
		modifiers.remove( "<SHIFT>" );
	}

	bool test = false;

	bool cindent = getLocalBoolOption( "cindent" );

	/** rightleft mapping **/
	if ( getLocalBoolOption( "rightleft" ) &&
		( mMode == YZ_VIEW_MODE_COMMAND || mMode == YZ_VIEW_MODE_VISUAL || mMode == YZ_VIEW_MODE_VISUAL_LINE )
	) {
#define SWITCH_KEY( a, b ) \
	if ( key == a ) key = b; \
	else if ( key == b ) key = a
		SWITCH_KEY( "<RIGHT>", "<LEFT>" );
		SWITCH_KEY( "h", "l" );
	}

	//check mappings
	if ( mMode == YZ_VIEW_MODE_INSERT || mMode == YZ_VIEW_MODE_REPLACE )
		mapMode = mapMode | insert;
	else if ( mMode == YZ_VIEW_MODE_COMMAND || mMode == YZ_VIEW_MODE_INTRO || mMode == YZ_VIEW_MODE_OPEN )
		mapMode = mapMode | normal;
	else if ( mMode == YZ_VIEW_MODE_VISUAL || mMode == YZ_VIEW_MODE_VISUAL_LINE )
		mapMode = mapMode | visual;
	else if ( mMode == YZ_VIEW_MODE_EX || mMode == YZ_VIEW_MODE_SEARCH )
		mapMode = mapMode | cmdline;

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
				if ( cindent ) {
					indent();
				} else {
					mBuffer->action()->insertNewLine( this, 0, mainCursor->bufferY() + 1 );
				}
				updateStickyCol( mainCursor );
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
					mBuffer->action()->mergeNextLine( this, mainCursor->bufferY() - 1 );
				} else if ( mainCursor->bufferX() > 0 )
					mBuffer->action()->deleteChar( this, mainCursor->bufferX() - 1, mainCursor->bufferY(), 1 );
				return;
			} else if ( key == "<DEL>" ) {
				mBuffer->action()->deleteChar( this, mainCursor->buffer(), 1 );
				commitNextUndo();
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
				//Vim has a sub mode for <CTRL>x commands
				if ( modifiers + key == "<CTRL>x" ) { //special handling, here we can run commands while in INSERT mode
					mPreviousChars += modifiers + key;
					return;
				} else if ( modifiers == "<CTRL>" && mPreviousChars == "<CTRL>x" ) {
					mPreviousChars += modifiers + key;
					cmd_state state=mSession->getPool()->execCommand(this, mPreviousChars);
					switch(state) {
						case CMD_ERROR:
						case CMD_OK:
							purgeInputBuffer();
							break;
						case OPERATOR_PENDING:
							mapMode = pendingop;
						default:
							break;
					}
					return;
				}
				mBuffer->action()->insertChar( this, mainCursor->buffer(), key );
				if ( cindent && key == "}" )
					reindent(mainCursor->bufferX()-1, mainCursor->bufferY());
				purgeInputBuffer(); //be safe in case we mistyped a CTRL command just before
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
				commitNextUndo();
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
				commitNextUndo();
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
				commitNextUndo();
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
				//Vim has a sub mode for <CTRL>x commands
				if ( modifiers + key == "<CTRL>x" ) { //special handling, here we can run commands while in INSERT mode
					mPreviousChars += modifiers + key;
					return;
				} else if ( modifiers == "<CTRL>" && mPreviousChars == "<CTRL>x" ) {
					mPreviousChars += modifiers + key;
					cmd_state state=mSession->getPool()->execCommand(this, mPreviousChars);
					switch(state) {
						case CMD_ERROR:
						case CMD_OK:
							purgeInputBuffer();
							break;
						default:
						break;
					}
					return;
				}
				mBuffer->action()->replaceChar( this, mainCursor->buffer(), key );
				commitNextUndo();
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
				mSession->getExPool()->execCommand( this, cmd );
				//we'll need to check that undo step ...
				//breaks when we quit ;), we'll check after the EX rework
				//commitNextUndo();
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

		//all these use Normal Commands in commands.cpp
		case YZ_VIEW_MODE_VISUAL:
		case YZ_VIEW_MODE_VISUAL_LINE :
		case YZ_VIEW_MODE_COMMAND:
			{
				mPreviousChars+=modifiers+key;

				QString mapped = mPreviousChars;
				bool pendingMapp = YZMapping::self()->applyMappings(mapped, mapMode);
				
				cmd_state state=mSession->getPool()->execCommand(this, mapped);
				switch(state) {
					case CMD_ERROR:
//						yzDebug() << "Error" << endl;
						if (pendingMapp) break;
					case CMD_OK:
						purgeInputBuffer();
						break;
					case OPERATOR_PENDING:
	//					yzDebug() << "Pending" << endl;
						mapMode = pendingop;
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
		case YZ_VIEW_MODE_INTRO:
			clearIntro();
			recalcScreen();
			gotoCommandMode();
			sendKey( _key, _modifiers );
			break;
		default:
			yzDebug() << "Unknown MODE" << endl;
			purgeInputBuffer();
	};
}

YZSelectionMap YZView::visualSelection() {
	return selectionPool->layout( "VISUAL" );
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

/*
* Right now indent just copies the leading whitespace of the current line into the
* new line this isn't smart as it will duplicate extraneous spaces in indentation
* rather than giving consistent depth changes/composition based on user settings.
*/
void YZView::indent() {
	//yzDebug() << "Entered YZView::indent" << endl;
	QString indentMarker = "{"; // Just use open brace for now user defined (BEGIN or whatever) later
	unsigned int ypos = mainCursor->bufferY();
	QString currentLine = mBuffer->textline( ypos );
	QRegExp rxLeadingWhiteSpace( "^([ \t]*).*$" );
	if ( !rxLeadingWhiteSpace.exactMatch( currentLine ) ) {
		return; //Shouldn't happen
	}
	QString indentString = rxLeadingWhiteSpace.cap( 1 );
	if ( mainCursor->bufferX() == currentLine.length() && currentLine.stripWhiteSpace().endsWith( indentMarker ) ) {
		//yzDebug() << "Indent marker found" << endl;
		// This should probably be tabstop...
		indentString.append( "\t" );
	}
	//yzDebug() << "Indent string = \"" << indentString << "\"" << endl;
	mBuffer->action()->insertNewLine( this, mainCursor->buffer() );
	ypos++;
	mBuffer->action()->replaceLine( this, ypos, indentString + mBuffer->textline( ypos ).stripWhiteSpace() );
	gotoxy( indentString.length(), ypos );
	//yzDebug() << "Leaving YZView::indent" << endl;
}

QString YZView::centerLine( QString s )
{
	QString spacer = "";
	unsigned int nspaces = mColumnsVis > s.length() ? mColumnsVis - s.length() : 0;
	nspaces /= 2;
	spacer.fill( ' ', nspaces );
	spacer.append( s );
	return spacer;
}

void YZView::displayIntro() {
	yzDebug() << "File: " __FILE__ << " Line: " << __LINE__ << endl;
	unsigned int linesInIntro = 11; // Update this is if you change # of lines in message
	unsigned int vMargin = mLinesVis > linesInIntro ? mLinesVis - linesInIntro : 0;
	vMargin = ( vMargin + 1 ) / 2; // round up to have enough lines so '~' isn't shown

	/* Don't record these in the undo list */
	mBuffer->undoBuffer()->setInsideUndo( true );

	gotoxy( 0, 0 );
	for (unsigned int i = 0; i < vMargin; i++ ) mBuffer->appendLine("");
	mBuffer->appendLine( centerLine( VERSION_CHAR_LONG ) );
	if ( VERSION_CHAR_ST == VERSION_CHAR_STATE2 )
		mBuffer->appendLine( centerLine( VERSION_CHAR_DATE ) );
	mBuffer->appendLine( centerLine( VERSION_CHAR_ST  ) );
	mBuffer->appendLine( "" );
	mBuffer->appendLine( centerLine( "http://www.yzis.org" ) );
	mBuffer->appendLine( centerLine( "contact/patches/requests: yzis-dev@yzis.org" ) );
	mBuffer->appendLine( "" );
	mBuffer->appendLine( centerLine( "Yzis is distributed under the terms of the GPL v2" ) );
	mBuffer->appendLine( "" );
	mBuffer->appendLine( centerLine( "please report bugs at http://bugs.yzis.org" ) );
	for ( unsigned int i = 0; i < vMargin; i++ ) mBuffer->appendLine( "" );

	mBuffer->undoBuffer()->setInsideUndo( false );

	gotoIntroMode();
	refreshScreen();
}

void YZView::clearIntro()
{
	yzDebug() << "Entered YZView::clearIntro" << endl;
	mBuffer->undoBuffer()->setInsideUndo( true );

	gotoxy( 0, 0 );
	mBuffer->clearText();

	mBuffer->undoBuffer()->setInsideUndo( false );

	mBuffer->setChanged( false );
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

void YZView::bottomViewVertically( unsigned int line ) {
	unsigned int newcurrent = 0;
	if ( line >= mLinesVis ) newcurrent = (line - mLinesVis) + 1;
	alignViewVertically( newcurrent );
}

void YZView::alignViewVertically( unsigned int line ) {
//	yzDebug() << "YZView::alignViewVertically " << line << endl;
	unsigned int newcurrent = line;
	unsigned int old_dCurrentTop = dCurrentTop;
//	yzDebug() << "newcurrent=" << newcurrent << "; alignTop=" << alignTop << "; old_dCurrentTop=" << dCurrentTop << endl;
	if ( wrap && newcurrent > 0 ) {
		initDraw();
		drawMode = false;
		gotody( newcurrent );
//		yzDebug() << "raw top = " << *sCursor << "; r=" << *rCursor << endl;
		// rLineHeight > 1 => our new top is in middle of a wrapped line, move new top to next line
		newcurrent = workCursor->bufferY();
		if ( workCursor->lineHeight > 1 ) 
			++newcurrent;
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
		abortPaintEvent();
		refreshScreen();
	}
}

/* recalculate cursor position + refresh screen */
void YZView::reset( ) {
	if ( getLocalStringOption( "encoding" ) != mBuffer->encoding() ) {
		mBuffer->setEncoding( getLocalStringOption( "encoding" ) );
	}
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

void YZView::gotox( unsigned int nextx, bool forceGoBehindEOL ) {
	if ( ( int )nextx < 0 ) nextx = 0;
	unsigned int shift = ( ( ! drawMode && ( YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT==mMode && sCurLineLength > 0 ) ) || forceGoBehindEOL ) ? 1 : 0;
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
				unsigned int wrapLineMinHeight = ( unsigned int ) ceil( rMinCurLineLength / mColumnsVis ) + 1;
				unsigned int wrapLineMaxHeight = ( unsigned int ) ceil( rCurLineLength / mColumnsVis ) + 1;
				if ( wrapLineMinHeight == wrapLineMaxHeight ) {
					workCursor->setScreenY( workCursor->screenY() + 1 - wrapLineMinHeight );
				} else {
					unsigned int cury = workCursor->bufferY();
					unsigned int prevRX = workCursor->screenY();
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
				unsigned int wrapLineMinHeight = ( unsigned int ) ceil( rMinCurLineLength / mColumnsVis ) + 1;
				unsigned int wrapLineMaxHeight = ( unsigned int ) ceil( rCurLineLength / mColumnsVis ) + 1;
				if ( wrapLineMinHeight == wrapLineMaxHeight ) {
					workCursor->setScreenY( workCursor->screenY() + 1 - wrapLineMinHeight );
				} else {
					unsigned int cury = workCursor->bufferY();
					unsigned int prevRX = workCursor->screenY();
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
	}

/*	yzDebug() << "applyGoto : "
			<< "dColLength=" << dColLength << "; dLineLength=" << dLineLength << "; mLineLength=" << mLineLength
			<< "; dWrapNextLine=" << dWrapNextLine << "; dWrapTab=" << dWrapTab << endl;
	yzDebug() << "mCursor:" << *mCursor << "; dCursor:" << *dCursor << endl; */

	if ( applyCursor ) {

		setPaintAutoCommit( false );
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
				dBegin.setX( 0 );
				dEnd.setX( mColumnsVis );
			}
			selectionPool->addSelection( "VISUAL", bBegin, bEnd, dBegin, dEnd );
//			yzDebug() << "visual selection : from " << bBegin << " to " << bEnd << endl;

			YZCursor dOldBegin( cur_sel.drawFrom() );
			YZCursor dOldEnd( cur_sel.drawTo() );

//			yzDebug() << "VISUAL MODE : old = " << dOldBegin << " -> " << dOldEnd << endl;
//			yzDebug() << "              new = " << dBegin << " -> " << dEnd << endl;

			sendPaintEvent( QMIN( dOldBegin, dBegin ), QMAX( dOldEnd, dEnd ) );
//			selectionPool->debug( "DRAW" );
			removePaintEvent( QMAX( dOldBegin, dBegin ), QMIN( dOldEnd, dEnd ) );
//			selectionPool->debug( "DRAW" );
		}

		if ( !isLineVisible( mainCursor->screenY() ) ) {
			if ( mainCursor->screenY() >= mLinesVis + dCurrentTop )
				bottomViewVertically( mainCursor->screenY() );
			else
				alignViewVertically( mainCursor->screenY() );
		}
		if ( !isColumnVisible( mainCursor->screenX(), mainCursor->screenY() ) ) {
			centerViewHorizontally( mainCursor->screenX( ) );
			abortPaintEvent();
			refreshScreen();
		}
		commitPaintEvent();
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
	gotox( nextx, viewCursor != mainCursor );
	applyGoto( viewCursor, applyCursor );
}

void YZView::moveXY( unsigned int x, unsigned int y ) {
	gotoxy( x, y );
	updateStickyCol( mainCursor );
}

QString YZView::moveDown( unsigned int nb_lines, bool applyCursor ) {
	return moveDown( mainCursor, nb_lines, applyCursor );
}
QString YZView::moveDown( YZViewCursor* viewCursor, unsigned int nb_lines, bool applyCursor ) {
	gotoStickyCol( viewCursor, QMIN( viewCursor->bufferY() + nb_lines, mBuffer->lineCount() - 1 ), applyCursor );
	return QString::null;
}
QString YZView::moveUp( unsigned int nb_lines, bool applyCursor ) {
	return moveUp( mainCursor, nb_lines, applyCursor );
}
QString YZView::moveUp( YZViewCursor* viewCursor, unsigned int nb_lines, bool applyCursor ) {
	gotoStickyCol( viewCursor, QMAX( viewCursor->bufferY() - nb_lines, 0 ), applyCursor );
	return QString::null;
}

QString YZView::moveLeft( int nb_cols, bool wrap, bool applyCursor ) {
	return moveLeft( mainCursor, nb_cols, wrap, applyCursor );
}

QString YZView::moveLeft( YZViewCursor* viewCursor, int nb_cols, bool wrap, bool applyCursor ) {
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

	if ( applyCursor ) updateStickyCol( viewCursor );

	//return something
	return QString::null;
}

QString YZView::moveRight( int nb_cols, bool wrap, bool applyCursor ) {
	return moveRight( mainCursor, nb_cols, wrap, applyCursor );
}

QString YZView::moveRight( YZViewCursor* viewCursor, int nb_cols, bool wrap, bool applyCursor ) {
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
			x=myBuffer()->textline(y).length();
	}
	gotoxy( viewCursor, (unsigned int)(x), y);

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
	//execute the code
	gotoxy(viewCursor, 0 , viewCursor->bufferY(), applyCursor);
//	if ( viewCursor == mainCursor )	UPDATE_STICKY_COL;
	if ( applyCursor )
		updateStickyCol( viewCursor );

	//return something
	return QString::null;
}

void YZView::gotoLastLine() {
	gotoLastLine( mainCursor );
}
void YZView::gotoLastLine( YZViewCursor* viewCursor, bool applyCursor ) {
	gotoLine( viewCursor, mBuffer->lineCount() - 1, applyCursor );
}

void YZView::gotoLine( unsigned int line ) {
	gotoLine( mainCursor, line );
}
void YZView::gotoLine( YZViewCursor* viewCursor, unsigned int line, bool applyCursor ) {
	if ( line >= mBuffer->lineCount() )
		line = mBuffer->lineCount() - 1;

	if ( getLocalBoolOption("startofline") ) {
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

void YZView::initChanges( unsigned int x, unsigned int y ) {
	beginChanges->setX( x );
	beginChanges->setY( y );
	origPos->setCursor( mainCursor->buffer() );
	if ( wrap ) {
		gotoxy( mBuffer->textline( y ).length(), y, false );
		lineDY = mainCursor->screenY();
	}
	gotoxy( x, y, false );
}

void YZView::applyChanges( unsigned int /*x*/, unsigned int y ) {
	unsigned int dY = mainCursor->screenY();
	if ( y != beginChanges->getY() ) {
		sendPaintEvent( dCurrentLeft, dY, mColumnsVis, mLinesVis - ( dY - dCurrentTop ) );
	} else {
		if ( wrap ) {
			gotoxy( mBuffer->textline( y ).length(), y, false );
			if ( mainCursor->screenY() != lineDY )
				sendPaintEvent( dCurrentLeft, dY, mColumnsVis, mLinesVis - ( dY - dCurrentTop ) );
			else
				sendPaintEvent( dCurrentLeft, dY, mColumnsVis, 1 + mainCursor->screenY() - dY );
		} else
			sendPaintEvent( dCurrentLeft, dY, mColumnsVis, 1 );
	}
	gotoxy( origPos->getX(), origPos->getY(), false );
}

/*QString YZView::deleteLine ( unsigned int nb_lines, const QValueList<QChar> &regs ) {
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
}*/

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

QString YZView::gotoIntroMode() {
	switchModes( YZ_VIEW_MODE_INTRO );
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
	sendPaintEvent( dCurrentLeft, dVisualCursor->getY(), mColumnsVis, 1 );
	yzDebug("Visual mode") << "Starting at " << *mVisualCursor << endl;
	return QString::null;
}

void YZView::leaveVisualMode( ) {
	YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
	selectionPool->clear( "VISUAL" );
	sendPaintEvent( dCurrentLeft, cur_sel.drawFrom().getY()/* > 0 ? cur_sel.drawFrom().getY() - 1 : 0*/, mColumnsVis, cur_sel.drawTo().getY() - cur_sel.drawFrom().getY() + 1 );
	gotoPreviousMode();
}

YZSelectionMap YZView::getVisualSelection() {
	return selectionPool->layout("VISUAL");
}

void YZView::paste( QChar registr, bool after ) {
	QStringList list = YZSession::mRegisters.getRegister( registr );
	if ( list.isEmpty() ) return;

	YZCursor pos( mainCursor->buffer() );
	uint i = 0;
	bool copyWholeLinesOnly = list[ 0 ].isNull();
	QString copy = mBuffer->textline( pos.getY() );
	if ( after || ! copyWholeLinesOnly ) { //paste after current char
		unsigned int start;
		if( after )
			start = copy.length() > 0 ? pos.getX() + 1 : 0;
		else
			start = pos.getX();
		i = 0;
		if ( ! copyWholeLinesOnly ) {
			copy = copy.mid( start );
			mBuffer->action()->deleteChar( this, start, pos.getY(), copy.length() );
			mBuffer->action()->insertChar( this, start, pos.getY(), list[ 0 ] + ( list.size() == 1 ? copy : "" ) );
			gotoxy( start + list[ 0 ].length() - ( list[ 0 ].length() > 0 ? 1 : 0 ), pos.getY() );
		}
		i++;
		while ( i < list.size() - 1 ) {
			mBuffer->action()->insertLine( this, pos.getY() + i, list[ i ] );
			i++;
		}
		if ( i < list.size() && ! copyWholeLinesOnly ) {
			mBuffer->action()->insertLine( this, pos.getY() + i, ( list[ i ].isNull() ? "" : list[ i ] ) + copy );
			gotoxy( list[ i ].length(), pos.getY() + i );
		} else if ( copyWholeLinesOnly ) {
			gotoxy( 0, pos.getY() + 1 );
			moveToFirstNonBlankOfLine();
		}

	} else if ( !after ) { //paste whole lines before current char
		for( i = 1; i < list.size() - 1; i++ )
			mBuffer->action()->insertLine( this, pos.getY() + i - 1, list[ i ] );

		gotoxy( pos.getX(), pos.getY() );
	}
	updateStickyCol( mainCursor );
}

bool YZView::doSearch( const QString& search ) {
	selectionPool->clear( "SEARCH" );

	bool found = false;
	unsigned int matchlength = 0;
	YZCursor begin = *mainCursor->buffer();
	YZCursor end = YZCursor (this, mBuffer->textline(mBuffer->lineCount()-1).length(), mBuffer->lineCount()-1);
	if ( reverseSearch ) {
		end.setX(0);
		end.setY(0);
	}
	YZCursor result;

	result = mBuffer->action()->search(this, search, begin, end, reverseSearch, &matchlength, &found);

	if ( found ) {
		selectionPool->addSelection( "SEARCH", result.getX(), result.getY(), result.getX() + matchlength - 1, result.getY() );
		gotoxy( result.getX(), result.getY() );
		updateStickyCol( mainCursor );
		refreshScreen( );
		return true;
	} else {
		refreshScreen( );
		return false;
	}
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
		if ( workCursor->sLineIncrement == 0 ) {
			workCursor->sLineIncrement = 1;
		}
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
/*			listChar = drawMode && getLocalBoolOption( "list" );
			if ( listChar ) {
				lastChar = '.';
				if ( stringHasOnlySpaces(sCurLine.mid(curx)) )
					lastChar = '-';
			}*/ // useless because drawPrevCol is not used to in drawing stuff in GUIs
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
		mFillChar = ' ';
		if ( drawMode ) charSelected = selectionPool->isSelected( workCursor->buffer() );
		if ( lastChar != tabChar ) {
			listChar = drawMode && getLocalBoolOption( "list" ) && lastChar == ' ';
			if ( listChar ) {
				YZInternalOption *opt = YZSession::mOptions.getOption("Global\\listchars");
				if (opt && stringHasOnlySpaces(sCurLine.mid(curx)) ) {
					QString option = opt->getValueForKey("trail");
					if ( !option.isNull() ) {
						lastChar = option[0];
					}
				} else if (opt) {
					QString option = opt->getValueForKey("space");
					if ( !option.isNull() ) {
						lastChar = option[0];
					}
				}
			}
			workCursor->sColIncrement = GET_CHAR_WIDTH( lastChar );
			lenToTest = workCursor->sColIncrement;
		} else {
			workCursor->lastCharWasTab = true;
			lastChar = ' ';
			listChar = getLocalBoolOption( "list" );
			if ( listChar ) {
				YZInternalOption *opt = YZSession::mOptions.getOption("Global\\listchars");
				if (opt) {
					QString option = opt->getValueForKey("tab");
					if ( !option.isNull() && option.length() > 0 ) {
						lastChar = option[0];
						if ( option.length() > 1 )
							mFillChar = option[1];
					}
				}
			}
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

const QChar& YZView::fillChar() const {
	return mFillChar;
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
	YzisAttribute *at = NULL;
	
	if ( yl->length() != 0 && highlight ) {
		hl = yl->attributes(); //attributes of this line
		hl += col; // -1 ? //move pointer to the correct column
		uint len = hl ? highlight->attributes( 0 )->size() : 0 ; //length of attributes
		YzisAttribute *list = highlight->attributes( 0 )->data( ); //attributes defined by the syntax highlighting document
		at = ( ( *hl ) >= len ) ? &list[ 0 ] : &list[*hl]; //attributes pointed by line's attribute for current column
	}
	if ( getLocalBoolOption( "list" ) && ( yl->data()[col] == ' ' || yl->data()[col] == tabChar ) )
		return blue;
	if ( at ) return at->textColor(); //textcolor :)
	return fake;
}

const QColor& YZView::drawColor ( ) {
	YzisAttribute hl;
	YzisAttribute * curAt = ( !rHLnoAttribs && (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt ) hl += * curAt;

	if ( listChar ) return blue;
	else if ( curAt ) return hl.textColor();
	else return fake;
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

void YZView::printToFile( const QString& path ) {
	if ( YZSession::getStringOption("printer") != "pslib" ) {
		if ( getenv( "DISPLAY" ) ) {
			YZQtPrinter qtprinter( this );
			qtprinter.printToFile( path );
			qtprinter.run( );
		} else {
			YZSession::me->popupMessage( tr("To use the Qt printer, you need to have an X11 DISPLAY set and running, you should try pslib in console mode") );
		}
		return;
	}

#ifdef HAVE_LIBPS
	YZPrinter printer( this );
	printer.printToFile( path );
	printer.run( );
#endif
}

void YZView::undo( unsigned int count ) {
	for ( unsigned int i = 0 ; i < count ; i++ )
		mBuffer->undoBuffer()->undo( this );
}

void YZView::redo( unsigned int count ) {
	for ( unsigned int i = 0 ; i < count ; i++ )
		mBuffer->undoBuffer()->redo( this );
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

void YZView::gotoStickyCol( unsigned int Y ) {
	gotoStickyCol( mainCursor, Y, true );
}

void YZView::gotoStickyCol( YZViewCursor* viewCursor, unsigned int Y, bool applyCursor ) {
	if ( stickyCol == STICKY_COL_ENDLINE )
		gotoxy( viewCursor, mBuffer->textline( Y ).length(), Y, applyCursor );
	else {
		unsigned int col = stickyCol % mColumnsVis;
		unsigned int deltaY = stickyCol / mColumnsVis;
		if ( deltaY == 0 ) {
			gotodxy( viewCursor, col, Y, applyCursor );
		} else {
			unsigned int lineLength = mBuffer->textline( Y ).length();
			gotoxy( viewCursor, 0, Y, false );
			unsigned int startDY = viewCursor->screenY();
			gotoxy( viewCursor, lineLength, Y, false );
			unsigned int endDY = viewCursor->screenY();
			if ( startDY + deltaY > endDY ) {
				gotoxy( viewCursor, lineLength, Y, applyCursor );
			} else {
				gotodxdy( viewCursor, col, startDY + deltaY, applyCursor );
			}
		}
	}
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

YZCursor *YZView::getCursor() {
	return mainCursor->screen();
}

YZCursor *YZView::getBufferCursor() {
	return mainCursor->buffer();
}

void YZView::recordMacro( const QValueList<QChar> &regs ) {
	mRegs = regs;
}

void YZView::stopRecordMacro() {
	for ( QValueList<QChar>::iterator it = mRegs.begin(); it != mRegs.end(); it++ ) {
		QStringList list;
		QString ne = YZSession::mRegisters.getRegister( *it )[ 0 ];
		list << ne.mid( 0, ne.length() - 1 ); //remove the last 'q' which was recorded ;)
		YZSession::mRegisters.setRegister( *it, list);
	}
	mRegs = QValueList<QChar>();
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
		YZSelectionMap drawPool = selectionPool->layout( "DRAW" );
		unsigned int size = drawPool.size();
		unsigned int lastY = 0;
		for ( unsigned int i = 0; i < size; i++ ) {
			unsigned int fY = drawPool[ i ].from().getY();
			unsigned int tY = drawPool[ i ].to().getY();
			if ( i > 0 && lastY == fY ) { // don't send two events on the same line
				if ( fY == tY ) continue;
				++fY;
			}
			lastY = tY;
			sendPaintEvent( dCurrentLeft, fY, mColumnsVis, tY - fY + 1 );
		}
		abortPaintEvent();
	}
}
void YZView::abortPaintEvent() {
	selectionPool->clear( "DRAW" );
	setPaintAutoCommit();
}

void YZView::sendPaintEvent( const YZCursor& from, const YZCursor& to ) {
	sendPaintEvent( from.getX(), from.getY(), to.getX() - from.getX(), to.getY() - from.getY() );
}
void YZView::sendPaintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh ) {
	if ( m_paintAutoCommit == 0 ) {
		paintEvent( curx, cury, curw, curh );
	} else {
		selectionPool->addSelection( "DRAW", curx, cury, curx + curw, cury + curh );
	}
}
void YZView::removePaintEvent( const YZCursor& from, const YZCursor& to ) {
	selectionPool->delSelection( "DRAW", from, to, from, to );
}

bool YZView::stringHasOnlySpaces ( const QString& what ) {
	for (unsigned int i = 0 ; i < what.length(); i++)
		if ( !what[i].isSpace() ) {
			return false;
		}
	return true;
}

QString YZView::mode ( int mode ) {
	if (isRecording()) {
		return mModes[mode] + tr(" { Recording }");
	}
	return mModes[mode];
}

