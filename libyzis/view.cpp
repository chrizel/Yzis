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

#include "portability.h"
#include <cstdlib>
#include <ctype.h>
#include <qkeysequence.h>
#include <qclipboard.h>
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
			<< tr("[ Open ]") << tr("[ Introduction ]") << tr("{ Completion }") << tr("[ Visual ]") << tr("[ Visual Line ]") << tr("Yzis Ready");
	mainCursor = new YZViewCursor( this );
	workCursor = new YZViewCursor( this );
	keepCursor = new YZViewCursor( this );

	scrollCursor = new YZViewCursor( this );

	origPos = new YZCursor( this );

	/* start of visual mode */
	mVisualCursor = new YZCursor(this);
	dVisualCursor = new YZCursor(this);

	mSearchBegin = new YZCursor( this );

	beginChanges = new YZCursor( this );

	incSearchFound = false;
	incSearchResult = new YZCursor( this );

	stickyCol = 0;

	mPrevMode = mMode = YZ_VIEW_MODE_COMMAND;
	mapMode = 0;

	QString line = mBuffer->textline(scrollCursor->bufferY());

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

	//completion
	m_completionStart = new YZCursor(this);
	m_completionCursor = new YZCursor(this);

	abortPaintEvent();
}

YZView::~YZView() {
	yzDebug() << "YZView : Deleting view " << myId << endl;
	mBuffer->rmView(this); //make my buffer forget about me

	delete mainCursor;
	delete scrollCursor;
	delete keepCursor;
	delete workCursor;
	delete origPos;
	delete mVisualCursor;
	delete dVisualCursor;
	delete selectionPool;
	delete beginChanges;
	delete m_completionStart;
	delete m_completionCursor;
	delete incSearchResult;
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
	if ( mBuffer->getLocalStringOption( "encoding" ) != mBuffer->encoding() ) {
		mBuffer->setEncoding( mBuffer->getLocalStringOption( "encoding" ) );
	}
	YZCursor old_pos = *scrollCursor->buffer();
	scrollCursor->reset();
	gotoxy( scrollCursor, old_pos.getX(), old_pos.getY(), false );

	old_pos = *mainCursor->buffer();
	mainCursor->reset();
	gotoxy( mainCursor, old_pos.getX(), old_pos.getY() );

	abortPaintEvent();
	refreshScreen();
}

void YZView::sendMultipleKey(const QString& _keys) {
	QString keys = _keys;
	yzDebug() << "sendMultipleKey " << keys << endl;
	for ( unsigned int i = 0 ; i < keys.length(); i++ ) {
		QString key = keys.mid( i );
		if ( key.startsWith( "<CTRL>" ) ) {
			yzDebug() << "Sending " << key.mid(6,1) << endl;
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
		QValueList<QChar>::iterator end = mRegs.end();
		for ( QValueList<QChar>::iterator it = mRegs.begin(); it != end; ++it ) {
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
	if ( mMode == YZ_VIEW_MODE_INSERT || mMode == YZ_VIEW_MODE_REPLACE || mMode == YZ_VIEW_MODE_COMPLETION )
		mapMode = mapMode | insert;
	else if ( mMode == YZ_VIEW_MODE_COMMAND || mMode == YZ_VIEW_MODE_INTRO || mMode == YZ_VIEW_MODE_OPEN )
		mapMode = mapMode | normal;
	else if ( mMode == YZ_VIEW_MODE_VISUAL || mMode == YZ_VIEW_MODE_VISUAL_LINE )
		mapMode = mapMode | visual;
	else if ( mMode == YZ_VIEW_MODE_EX || mMode == YZ_VIEW_MODE_SEARCH )
		mapMode = mapMode | cmdline;

	bool pendingMapp = false;
	bool map = false;
	cmd_state state;
	QString temp;

	switch(mMode) {
		case YZ_VIEW_MODE_COMPLETION:
			temp = modifiers + key;
			pendingMapp = YZMapping::self()->applyMappings(temp, mapMode, &map);
			//the result of a mapping could be multiple keys so we can't parse them directly
			if ( map ) {
				QString n = temp;
				purgeInputBuffer();
				sendMultipleKey(n);
				return;
			}
			if ( modifiers+key == "<CTRL>p" ) {
				if (m_word2Complete.isEmpty())
					initCompletion();
				QString result = doComplete(false);
				if (!result.isNull()) {
					myBuffer()->action()->replaceText(this, *m_completionStart, mainCursor->bufferX()-m_completionStart->getX(), result);
					gotoxy(m_completionStart->getX()+result.length(),mainCursor->bufferY());
				}
				purgeInputBuffer();
				return;
			} else if ( modifiers+key == "<CTRL>n" ) {
				if (m_word2Complete.isEmpty())
					initCompletion();
				QString result = doComplete(true);
				if (!result.isNull()) {
					myBuffer()->action()->replaceText(this, *m_completionStart, mainCursor->bufferX()-m_completionStart->getX(), result);
					gotoxy(m_completionStart->getX()+result.length(),mainCursor->bufferY());
				}
				purgeInputBuffer();
				return;
			} else if ( modifiers+key == "<CTRL>x" ) {
				yzDebug() << "Skip CTRLx in completion mode" << endl;
				purgeInputBuffer();
				return;
			} else if ( modifiers+key == "<ESC>" ) {
				myBuffer()->action()->replaceText(this, *m_completionStart, mainCursor->bufferX()-m_completionStart->getX(), m_word2Complete);
				gotoxy(m_completionStart->getX()+m_word2Complete.length(),mainCursor->bufferY());
				leaveCompletionMode();
				purgeInputBuffer();
				return;
			} else {
				leaveCompletionMode();
			}
		case YZ_VIEW_MODE_INSERT:
			mPreviousChars += modifiers + key;
			pendingMapp = YZMapping::self()->applyMappings(mPreviousChars, mapMode, &map);
			//the result of a mapping could be multiple keys so we can't parse them directly
			if ( map ) {
				QString n = mPreviousChars;
				purgeInputBuffer();
				sendMultipleKey(n);
				return;
			}
			if ( mPreviousChars == "<HOME>" ) {
				moveToStartOfLine( );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<END>" ) {
				moveToEndOfLine( );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<INS>" ) {
				gotoReplaceMode( );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<ALT>:" ) {
				leaveInsertMode();
				gotoCommandMode();
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<ESC>" ) {
				leaveInsertMode();
				gotoPreviousMode();
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<ENTER>" ) {
				if ( cindent ) {
					indent();
				} else {
					mBuffer->action()->insertNewLine( this, mainCursor->buffer() );
					QStringList results = YZSession::events->exec("INDENT_ON_ENTER", this);
					if (results.count() > 0 ) {
						if (results[0].length()!=0) {
							mBuffer->action()->replaceLine( this, mainCursor->bufferY(), results[0] + mBuffer->textline( mainCursor->bufferY() ).stripWhiteSpace() );
							gotoxy(results[0].length(),mainCursor->bufferY());
						}
					}
				}
				updateStickyCol( mainCursor );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<DOWN>" ) {
				moveDown( );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<LEFT>" ) {
				moveLeft();
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<RIGHT>" ) {
				moveRight();
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<UP>" ) {
				moveUp( );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<BS>" ) {
				if (mainCursor->bufferX() == 0 && mainCursor->bufferY() > 0 && getLocalStringOption( "backspace" ).contains( "eol" ) ) {
					mBuffer->action()->mergeNextLine( this, mainCursor->bufferY() - 1 );
				} else if ( mainCursor->bufferX() > 0 )
					mBuffer->action()->deleteChar( this, mainCursor->bufferX() - 1, mainCursor->bufferY(), 1 );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<DEL>" ) {
				mBuffer->action()->deleteChar( this, mainCursor->buffer(), 1 );
				commitNextUndo();
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<PDOWN>" ) {
				gotoStickyCol( mainCursor, mainCursor->bufferY() + mLinesVis );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<PUP>" ) {
				gotoStickyCol( mainCursor, mainCursor->bufferY() > mLinesVis ? mainCursor->bufferY() - mLinesVis : 0 );
				purgeInputBuffer();
				return;
			} else {
				if ( mPreviousChars == "<TAB>" ) {
					mPreviousChars="\t";
				}
				if ( mPreviousChars == "<CTRL>x" ) {
					gotoCompletionMode();
					purgeInputBuffer();
					return;
				}
				if ( mPreviousChars.startsWith("<CTRL>") ) {
					state=mSession->getPool()->execCommand(this, mPreviousChars);
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
				if (!pendingMapp) {
					mBuffer->action()->insertChar( this, mainCursor->buffer(), mPreviousChars );
					if ( cindent && mPreviousChars == "}" )
						reindent(mainCursor->bufferX()-1, mainCursor->bufferY());
					QStringList ikeys = mBuffer->getLocalStringListOption("indentkeys");
					if ( ikeys.contains(mPreviousChars) )
						YZSession::events->exec("INDENT_ON_KEY", this);
					purgeInputBuffer(); //be safe in case we mistyped a CTRL command just before
				}
				return;
			}
			break;

		case YZ_VIEW_MODE_REPLACE:
			mPreviousChars += modifiers + key;
			pendingMapp = YZMapping::self()->applyMappings(mPreviousChars, mapMode, &map);
			if ( map ) {
				QString n = mPreviousChars;
				purgeInputBuffer();
				sendMultipleKey(n);
				return;
			}
			if ( mPreviousChars == "<HOME>" ) {
				moveToStartOfLine( );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<END>" ) {
				moveToEndOfLine( );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<INS>" ) {
				gotoInsertMode( );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<DEL>" ) {
				mBuffer->action()->deleteChar( this, mainCursor->buffer(), 1 );
				commitNextUndo();
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<ESC>" ) {
				leaveReplaceMode();
				gotoPreviousMode();
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<RETURN>" ) {
				test = mainCursor->bufferX() == 0;
				mBuffer->action()->insertNewLine( this, mainCursor->buffer() );
				if ( test ) {
					gotoxy( 0, mainCursor->bufferY() + 1 );
					updateStickyCol( mainCursor );
				}
				commitNextUndo();
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<DOWN>" ) {
				moveDown( );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<BS>" || mPreviousChars == "<LEFT>" ) {
				moveLeft();
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<RIGHT>" ) {
				moveRight();
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<UP>" ) {
				moveUp( );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<TAB>" ) {
				mBuffer->action()->replaceChar( this, mainCursor->buffer(), "\t" );
				commitNextUndo();
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<PDOWN>" ) {
				gotoStickyCol( mainCursor, mainCursor->bufferY() + mLinesVis );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars == "<PUP>" ) {
				gotoStickyCol( mainCursor, mainCursor->bufferY() > mLinesVis ? mainCursor->bufferY() - mLinesVis : 0 );
				purgeInputBuffer();
				return;
			} else if ( mPreviousChars.startsWith("<CTRL>x") ) {
				gotoCompletionMode();	
				purgeInputBuffer();
				return;
			} else {
				if ( mPreviousChars.startsWith("<CTRL>") ) {
					state=mSession->getPool()->execCommand(this, mPreviousChars);
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
				if (!pendingMapp) {
					mBuffer->action()->replaceChar( this, mainCursor->buffer(), mPreviousChars );
					commitNextUndo();
					purgeInputBuffer();
				}
				return;
			}
			break;

		case YZ_VIEW_MODE_SEARCH:
		{
			if ( key == "<ENTER>" ) {
				yzDebug() << "Current search : " << getCommandLineText();

				bool found = false;
				YZCursor pos;
				if ( getCommandLineText().isEmpty() ) {
					if ( reverseSearch ) {
						pos = YZSession::me->search()->replayBackward( this, &found );
					} else {
						pos = YZSession::me->search()->replayForward( this, &found );
					}
				} else {
					mSearchHistory[ mCurrentSearchItem++ ] = getCommandLineText();
					if ( reverseSearch ) {
							gotoxy( mainCursor->bufferX() + 1, mainCursor->bufferY(), false );
						pos = YZSession::me->search()->backward( this, getCommandLineText(), &found );
					} else {
						pos = YZSession::me->search()->forward( this, getCommandLineText(), &found );
					}
					if ( getLocalBoolOption( "incsearch" ) && incSearchFound ) {
						pos = *incSearchResult;
						incSearchFound = false;
					}
				}
				if ( found ) {
					gotoxy( pos.getX(), pos.getY() );
				} else {
					displayInfo(tr("No match"));
				}
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
				gotoxy(mSearchBegin->getX(), mSearchBegin->getY());
				if ( getLocalBoolOption( "incsearch" ) ) {
					incSearchFound = false;
					bool wasEmpty = selectionPool->layout( "SEARCH" ).isEmpty();
					selectionPool->clear( "SEARCH" );
					bool isEmpty = selectionPool->layout( "SEARCH" ).isEmpty();
					if ( ! ( wasEmpty && isEmpty ) )
						refreshScreen();
				}
				gotoPreviousMode();
				return;
			} else if ( key == "<BS>" ) {
				QString back = getCommandLineText();
				setCommandLineText(back.remove(back.length() - 1, 1));
			} else {
				setCommandLineText( getCommandLineText() + key );
			}

			if ( getLocalBoolOption("incsearch") ) {
				YZCursor end = YZCursor( this, 0, 0 );
				if ( ! reverseSearch ) {
					end.setY( mBuffer->lineCount() - 1 );
					end.setX( mBuffer->textline( end.getY() ).length() );
				}
				bool wasEmpty = selectionPool->layout( "SEARCH" ).isEmpty();
				unsigned int matchlength;
				incSearchResult->setCursor( mBuffer->action()->search( this, getCommandLineText(), mSearchBegin, end, 
										reverseSearch, &matchlength, &incSearchFound ) );
				if ( incSearchFound ) {
					if ( getLocalBoolOption("hlsearch") )
						selectionPool->addSelection( "SEARCH", incSearchResult->getX(), incSearchResult->getY(), 
									incSearchResult->getX() + matchlength - 1, incSearchResult->getY() );
					gotoxy( incSearchResult->getX(), incSearchResult->getY() );
					updateStickyCol( mainCursor );
				} else {
					gotoxy( mSearchBegin->getX(), mSearchBegin->getY() );
					selectionPool->clear( "SEARCH" );
				}
				bool isEmpty = selectionPool->layout( "SEARCH" ).isEmpty();
				if ( ! ( wasEmpty && isEmpty ) ) {
					refreshScreen( );
				}
			}
			break;
		}

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

				pendingMapp = YZMapping::self()->applyMappings(mPreviousChars, mapMode, &map);
				if ( map ) {
					QString n = mPreviousChars;
					purgeInputBuffer();
					sendMultipleKey(n);
					return;
				}
				
				cmd_state state=mSession->getPool()->execCommand(this, mPreviousChars);
				switch(state) {
					case CMD_ERROR:
						if (pendingMapp) break;
					case CMD_OK:
						purgeInputBuffer();
						break;
					case OPERATOR_PENDING:
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

QString YZView::centerLine( const QString& s ) {
	QString spacer = "";
	unsigned int nspaces = mColumnsVis > s.length() ? mColumnsVis - s.length() : 0;
	nspaces /= 2;
	spacer.fill( ' ', nspaces );
	spacer.append( s );
	return spacer;
}

void YZView::displayIntro() {
	yzDebug() << "File: " __FILE__ << " Line: " << __LINE__ << endl;
	unsigned int i;
	unsigned int linesInIntro = 11; // Update this is if you change # of lines in message
	unsigned int vMargin = mLinesVis > linesInIntro ? mLinesVis - linesInIntro : 0;
	vMargin = ( vMargin + 1 ) / 2; // round up to have enough lines so '~' isn't shown

	/* Don't record these in the undo list */
	mBuffer->undoBuffer()->setInsideUndo( true );

	gotoxy( 0, 0 );
	for (i = 0; i < vMargin; i++ ) mBuffer->appendLine("");
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
	for ( i = 0; i < vMargin; i++ ) mBuffer->appendLine( "" );

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
		if ( scrollCursor->bufferY() < 1 )  viewInformation.percentage=tr( "Top" );
		if ( scrollCursor->bufferY()+mLinesVis >= nblines )  viewInformation.percentage=tr( "Bot" );
		if ( (scrollCursor->bufferY()<1 ) &&  ( scrollCursor->bufferY()+mLinesVis >= nblines ) ) viewInformation.percentage=tr( "All" );
		lasty=y;
	}

	viewInformation.l = y;
	viewInformation.c1 = mainCursor->bufferX();
	viewInformation.c2 = mainCursor->screenX(); // XXX pas du tout, c'est c1 mais en remplacant les tabs par 'tablenght' <-- avec le QRegexp() mais je l'ai perdu

	syncViewInfo();
}

/**
 * TODO: scrollCursor can be separated from the buffer...
 */
void YZView::centerViewHorizontally(unsigned int column) {
//	yzDebug() << "YZView::centerViewHorizontally " << column << endl;
	unsigned int newcurrentLeft = 0;
	if ( column > mColumnsVis/2 ) newcurrentLeft = column - mColumnsVis / 2;

	if (newcurrentLeft > 0) {
		gotodxy( scrollCursor, newcurrentLeft, scrollCursor->bufferY() );
	} else {
		scrollCursor->reset();
	}
//	yzDebug() << "YZView::centerViewHorizontally : scrollCursor->screenX(): " << scrollCursor->screenX() << ", scrollCursor->bufferX(): " << scrollCursor->bufferX() << endl;
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

void YZView::alignViewBufferVertically( unsigned int line ) {
//	yzDebug() << "YZView::alignViewBufferVertically " << line << endl;
	unsigned int newcurrent = line;
	unsigned int old_dCurrentTop = scrollCursor->screenY();
	if ( newcurrent > 0 ) {
		gotodxy( scrollCursor, scrollCursor->screenX(), newcurrent );
		// TODO: handle correctly scrolling up/down with screenX > 0
	} else {
		scrollCursor->reset();
	}
	if ( old_dCurrentTop > scrollCursor->screenY() && old_dCurrentTop - scrollCursor->screenY() < mLinesVis ) {
		scrollUp( old_dCurrentTop - scrollCursor->screenY() );
	} else if ( old_dCurrentTop < scrollCursor->screenY() && scrollCursor->screenY() - old_dCurrentTop < mLinesVis ) {
		scrollDown( scrollCursor->screenY() - old_dCurrentTop );
	} else {
		abortPaintEvent();
		refreshScreen();
	}
}

void YZView::alignViewVertically( unsigned int line ) {
//	yzDebug() << "YZView::alignViewVertically " << line << endl;
	unsigned int newcurrent = line;
	unsigned int screenX = scrollCursor->screenX();
	unsigned int old_dCurrentTop = scrollCursor->screenY();
	if ( newcurrent > 0 ) {
		initGoto( scrollCursor );
		gotody( newcurrent );
		// rLineHeight > 1 => our new top is in middle of a wrapped line, move new top to next line
		if ( wrap ) {
			newcurrent = workCursor->bufferY();
			if ( workCursor->lineHeight > 1 ) 
				++newcurrent;
			gotoy( newcurrent );
		}
		gotodx( screenX );
		// TODO: handle correctly scrolling up/down with screenX > 0
		applyGoto( scrollCursor, false );
	} else {
		scrollCursor->reset();
	}
	if ( old_dCurrentTop > scrollCursor->screenY() && old_dCurrentTop - scrollCursor->screenY() < mLinesVis ) {
		scrollUp( old_dCurrentTop - scrollCursor->screenY() );
	} else if ( old_dCurrentTop < scrollCursor->screenY() && scrollCursor->screenY() - old_dCurrentTop < mLinesVis ) {
		scrollDown( scrollCursor->screenY() - old_dCurrentTop );
	} else {
		abortPaintEvent();
		refreshScreen();
	}
}

/*
 * all the goto-like commands
 */

/* PRIVATE */
void YZView::gotodx( unsigned int nextx ) {
	if ( ( int )nextx < 0 ) nextx = 0;
	unsigned int shift = ( ! drawMode && ( (YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT==mMode || YZ_VIEW_MODE_COMPLETION==mMode) && sCurLineLength > 0 ) ) ? 0 : 1;
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
	unsigned int shift = ( ( ! drawMode && ( (YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT==mMode || YZ_VIEW_MODE_COMPLETION==mMode) && sCurLineLength > 0 ) ) || forceGoBehindEOL ) ? 1 : 0;
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
	} else if ( nexty == scrollCursor->bufferY() ) {
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
#ifndef WIN32
			if ( QPaintDevice::x11AppDisplay() )
#endif
				QApplication::clipboard()->setText( mBuffer->getText( bBegin, bEnd ).join( "\n" ), QClipboard::Selection );

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
			if ( mainCursor->screenY() >= mLinesVis + scrollCursor->screenY() )
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
		sendPaintEvent( scrollCursor->screenX(), dY, mColumnsVis, mLinesVis - ( dY - scrollCursor->screenY() ) );
	} else {
		if ( wrap ) {
			gotoxy( mBuffer->textline( y ).length(), y, false );
			if ( mainCursor->screenY() != lineDY )
				sendPaintEvent( scrollCursor->screenX(), dY, mColumnsVis, mLinesVis - ( dY - scrollCursor->screenY() ) );
			else
				sendPaintEvent( scrollCursor->screenX(), dY, mColumnsVis, 1 + mainCursor->screenY() - dY );
		} else
			sendPaintEvent( scrollCursor->screenX(), dY, mColumnsVis, 1 );
	}
	gotoxy( origPos->getX(), origPos->getY(), false );
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
	else if (getCurrentMode()==YZ_VIEW_MODE_COMPLETION)
	{
		if (getPreviousMode() == YZ_VIEW_MODE_INSERT) gotoInsertMode();
		if (getPreviousMode() == YZ_VIEW_MODE_REPLACE) gotoReplaceMode();
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
 	mSearchBegin->setCursor( mainCursor->buffer() );
	return QString::null;
}

QString YZView::gotoIntroMode() {
	switchModes( YZ_VIEW_MODE_INTRO );
	return QString::null;
}

QString YZView::gotoCompletionMode() {
	switchModes( YZ_VIEW_MODE_COMPLETION );
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
	sendPaintEvent( scrollCursor->screenX(), dVisualCursor->getY(), mColumnsVis, 1 );
	yzDebug("Visual mode") << "Starting at " << *mVisualCursor << endl;
	return QString::null;
}

void YZView::leaveInsertMode( ) {
	if ( mainCursor->bufferX() > 0) moveLeft();
}

void YZView::leaveReplaceMode( ) {
	if ( mainCursor->bufferX() == mBuffer->textline( mainCursor->bufferY() ).length() )
		moveLeft( );
}

void YZView::leaveVisualMode( ) {
	YZSelection cur_sel = selectionPool->layout( "VISUAL" )[ 0 ];
	selectionPool->clear( "VISUAL" );
	sendPaintEvent( scrollCursor->screenX(), cur_sel.drawFrom().getY()/* > 0 ? cur_sel.drawFrom().getY() - 1 : 0*/, mColumnsVis, cur_sel.drawTo().getY() - cur_sel.drawFrom().getY() + 1 );
	gotoPreviousMode();
}

void YZView::leaveCompletionMode() {
	m_word2Complete = "";
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

/*
 * Drawing engine
 */

void YZView::setFixedFont( bool fixed ) {
	isFontFixed = fixed;
	spaceWidth = GET_CHAR_WIDTH( ' ' );
}

bool YZView::isColumnVisible( unsigned int column, unsigned int  ) {
	return ! (column < scrollCursor->screenX() || column >= (scrollCursor->screenX() + mColumnsVis));
}
bool YZView::isLineVisible( unsigned int l ) { 
	return ( ( l >= scrollCursor->screenY() ) && ( l < mLinesVis + scrollCursor->screenY() ) );
}
unsigned int YZView::getCurrentTop() { 
	return scrollCursor->bufferY(); 
}
unsigned int YZView::getDrawCurrentTop() {
	return scrollCursor->screenY();
}
unsigned int YZView::getCurrentLeft() { 
	return scrollCursor->bufferX();
}
unsigned int YZView::getDrawCurrentLeft() {
	return scrollCursor->screenX();
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
	initDraw( scrollCursor->bufferX(), scrollCursor->bufferY(), scrollCursor->screenX(), scrollCursor->screenY() );
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

	wrap = getLocalBoolOption( "wrap" );

	if (getLocalIntOption( "tabstop" ) != 0 ) // avoid division by zero
		tabstop = getLocalIntOption("tabstop");
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
				if ( scrollCursor->bufferX() > 0 )
					workCursor->spaceFill = ( tablength - scrollCursor->bufferX() % tablength ) % tablength;
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
	unsigned int shift = ( ! drawMode && ( (YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT==mMode || YZ_VIEW_MODE_COMPLETION==mMode) && sCurLineLength > 0 ) ) ? 1 : 0;
	if ( workCursor->bufferX() >= workCursor->bColIncrement ) {
		unsigned int curx = workCursor->bufferX() - 1;
		workCursor->setBufferX( curx );
		lastChar = sCurLine.at( curx );
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
		lastChar = sCurLine.at( curx );
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
			if ( workCursor->screenX( ) == scrollCursor->bufferX() )
				workCursor->sColIncrement = ( workCursor->spaceFill ? workCursor->spaceFill : tablength );
			else {
				// calculate tab position
				unsigned int mySpaceFill = ( scrollCursor->bufferX() == 0 ? workCursor->spaceFill : 0 );	// do not care about rSpaceFill if we arent't in wrapped mode
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
	}

	// can we go after the end of line buffer ?
	unsigned int shift = ( ! drawMode && ( (YZ_VIEW_MODE_REPLACE == mMode || YZ_VIEW_MODE_INSERT == mMode || YZ_VIEW_MODE_COMPLETION==mMode ) && sCurLineLength > 0 ) ) ? 1 : 0;

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

//	if ( drawMode && scrollCursor->bufferX() > 0 )
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
	if ( getLocalBoolOption( "list" ) && ( yl->data().at(col) == ' ' || yl->data().at(col) == tabChar ) )
		return blue;
	if ( at ) return at->textColor(); //textcolor :)
	return fake;
}

const QColor& YZView::drawColor ( ) {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

	if ( listChar ) return blue; //XXX make custom
	else if ( curAt ) return (*curAt).textColor();
	else return fake;
}

const QColor& YZView::drawSelColor ( ) {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

	if ( listChar ) return fake; //XXX make custom
	else if ( curAt ) return (*curAt).selectedTextColor();
	else return fake;
}

const QColor& YZView::drawBgColor ( ) {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

	if ( listChar ) return fake; //XXX make custom
	else if ( curAt ) return (*curAt).bgColor();
	else return fake;
}

const QColor& YZView::drawBgSelColor ( ) {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

	if ( listChar ) return fake; //XXX make custom
	else if ( curAt ) return (*curAt).selectedBGColor();
	else return fake;
}

bool YZView::drawBold() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt )
		return (*curAt).bold();
	return false;	
}

bool YZView::drawItalic() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt )
		return (*curAt).italic();
	return false;	
}

bool YZView::drawUnderline() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt )
		return (*curAt).underline();
	return false;	
}

bool YZView::drawOverline() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt )
		return (*curAt).overline();
	return false;	
}

bool YZView::drawStrikeOutLine() {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];
	if ( curAt )
		return (*curAt).strikeOut();
	return false;	
}

const QColor& YZView::drawOutline ( ) {
	curAt = ( rHLnoAttribs || (*rHLa) >= rHLAttributesLen ) ?  &rHLAttributes[ 0 ] : &rHLAttributes[*rHLa];

	if ( listChar ) return fake; //XXX make custom
	else if ( curAt ) return (*curAt).outline();
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
	QValueList<QChar>::iterator end = mRegs.end();
	for ( QValueList<QChar>::iterator it = mRegs.begin(); it != end; ++it ) {
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
		if ( keepCursor->valid() ) {
			*mainCursor = *keepCursor;
			keepCursor->invalidate();
			applyGoto( mainCursor );
		}
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
			sendPaintEvent( scrollCursor->screenX(), fY, mColumnsVis, tY - fY + 1 );
		}
		abortPaintEvent();
	}
}
void YZView::abortPaintEvent() {
	keepCursor->invalidate();
	selectionPool->clear( "DRAW" );
	setPaintAutoCommit();
}

void YZView::sendCursor( YZViewCursor* cursor ) {
	*keepCursor = *cursor;
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
		if ( !what.at(i).isSpace() ) {
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

void YZView::initCompletion() {
	YZNewMotionArgs arg (this, 1);
	YZCursor begin = YZSession::me->getPool()->moveWordBackward( arg );
	m_completionStart->setCursor(begin);
	YZCursor stop (this,mainCursor->bufferX()-1, mainCursor->bufferY());
	QStringList list = myBuffer()->getText(begin, stop);
	yzDebug() << "Completing word : " << list[0] << endl;
	//record current begin-of-word-to-complete
	m_word2Complete = list[0];
	m_completionCursor->setCursor(mainCursor->buffer());
	m_oldProposals.clear();
	m_lastCompletionDir = true;
}

QString YZView::doComplete(bool forward) {
	YZCursor result;
	unsigned int matchedLength=0;
	bool found=false;
	QString list="";
	
	if (m_lastCompletionDir != forward) {
		m_oldProposals.clear();
		m_lastCompletionDir = forward;
	}
	
	do {
		if (forward) {
			result = myBuffer()->action()->search(this, m_word2Complete+"\\w*", *m_completionCursor, YZCursor(this, 0, myBuffer()->lineCount()+1), false, &matchedLength, &found);
		} else {
			if ( *m_completionCursor == mainCursor->buffer() )
				m_completionCursor->setX(mainCursor->bufferX() - m_word2Complete.length());
			result = myBuffer()->action()->search(this, m_word2Complete+"\\w*", *m_completionCursor, YZCursor(this, 0, 0), true, &matchedLength, &found);
		}
		if (found) {
			YZCursor end (this, result.getX()+matchedLength-1, result.getY());
			list = myBuffer()->getText(result, end)[0];
			//yzDebug() << "Got testing match : " << list << " at " << result << " to " << end << endl;
			m_completionCursor->setCursor(result);
			if (forward) {
				if ( m_completionCursor->getX() < mBuffer->textline(m_completionCursor->getY()).length() )
					m_completionCursor->setX(m_completionCursor->getX()+1);
				else {
					m_completionCursor->setY(m_completionCursor->getY()+1);
					m_completionCursor->setX(0);
				}
			}
		}
	} while ( found && ( list == m_lastMatch || m_oldProposals.contains(list)) );

	//found something ?
	if ( found )  {
		yzDebug() << "Match : " << list << endl;
		m_lastMatch = list;
		m_oldProposals << list;
		return list;
	}
	//no more result clear the list if we want to go for another round
	m_oldProposals.clear();
	return QString::null;
}

