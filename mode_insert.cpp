/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
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

#include "mode_insert.h"
#include "portability.h"

#include "buffer.h"
#include "session.h"
#include "action.h"
#include "viewcursor.h"
#include "mode_complete.h"
#include "view.h"

#define dbg()    yzDebug("YZModeInsert")
#define err()    yzError("YZModeInsert")

using namespace yzis;

YZModeInsert::YZModeInsert() : YZMode() {
	mType = YZMode::MODE_INSERT;
	mString = _( "[ Insert ]" );
	mEditMode = true;
	mIM = true;
	mMapMode = insert;
}
void YZModeInsert::leave( YZView* mView ) {
	if ( mView->getBufferCursor().x() > 0 )
		mView->moveLeft();
}

void YZModeInsert::initModifierKeys() {
	mModifierKeys << "<CTRL>c" <<"<CTRL>e" << "<CTRL>n" << "<CTRL>p" << "<CTRL>x" << "<CTRL>y" << "<ALT>:" << "<ALT>v";
}
/*
 * if you add a command which use modifiers keys, add it in initModifierKeys too
 */
cmd_state YZModeInsert::execCommand( YZView* mView, const QString& _key ) {
	QString key = _key;
	cmd_state ret = CMD_OK;
	     if ( key == "<HOME>" ) commandHome( mView, key );
	else if ( key == "<END>" ) commandEnd( mView, key );
	else if ( key == "<ESC>" || key == "<CTRL>c" ) commandEscape( mView, key );
	else if ( key == "<INS>" ) commandInsert( mView, key );
	else if ( key == "<ALT>:" ) commandEx( mView, key );
	else if ( key == "<ALT>v" ) commandVisual( mView, key );
	else if ( key == "<DOWN>" ) commandDown( mView, key );
	else if ( key == "<LEFT>" ) commandLeft( mView, key );
	else if ( key == "<RIGHT>" ) commandRight( mView, key );
	else if ( key == "<UP>" ) commandUp( mView, key );
	else if ( key == "<PDOWN>" ) commandPageDown( mView, key );
	else if ( key == "<PUP>" ) commandPageUp( mView, key );
	else if	( key == "<CTRL>e" ) commandInsertFromBelow( mView, key );
	else if	( key == "<CTRL>y" ) commandInsertFromAbove( mView, key );
	// completion
	else if ( key == "<CTRL>x" ) commandCompletion( mView, key );
	else if ( key == "<CTRL>n" ) commandCompletionNext( mView, key );
	else if ( key == "<CTRL>p" ) commandCompletionPrevious( mView, key );
	else if ( key == "<BS>" ) commandBackspace( mView, key );
	else if ( key == "<ENTER>" ) commandEnter( mView, key );
	else if ( key == "<DEL>" ) commandDel( mView, key );
	else {
		if ( key == "<TAB>" ) {
			// expand a tab to [tabstop] spaces if 'expandtab' is set
			if (mView->getLocalBooleanOption("expandtab"))
				key.fill(' ', mView->getLocalIntegerOption("tabstop"));
			else
				key = "\t";
		}
		/* if ( key.startsWith("<CTRL>") ) // XXX no sense
			ret = YZSession::self()->getCommandPool()->execCommand(mView, key);
		else*/
		ret = commandDefault( mView, key );
		QStringList ikeys = mView->myBuffer()->getLocalListOption("indentkeys");
		if ( ikeys.contains(key) )
			YZSession::self()->eventCall("INDENT_ON_KEY", mView);
	}
	return ret;
}

void YZModeInsert::commandHome( YZView* mView, const QString& ) {
	mView->moveToStartOfLine();
}
void YZModeInsert::commandEnd( YZView* mView, const QString& ) {
	mView->moveToEndOfLine();
}
void YZModeInsert::commandEscape( YZView* mView, const QString& ) {
	mView->modePool()->pop( MODE_COMMAND );
}
void YZModeInsert::commandInsert( YZView* mView, const QString& ) {
	mView->modePool()->change( MODE_REPLACE, false );
}
void YZModeInsert::commandEx( YZView* mView, const QString& ) {
	mView->modePool()->push( MODE_EX );
}
void YZModeInsert::commandVisual( YZView* mView, const QString& ) {
	mView->modePool()->push( MODE_VISUAL );
}
void YZModeInsert::commandInsertFromAbove( YZView* mView, const QString& ) {
	QString c = mView->getCharBelow( -1 );
	if ( ! c.isNull() )
		commandDefault( mView, c );
}
void YZModeInsert::commandInsertFromBelow( YZView* mView, const QString& ) {
	QString c = mView->getCharBelow( 1 );
	if ( ! c.isNull() )
		commandDefault( mView, c );
}
void YZModeInsert::commandCompletion( YZView* mView, const QString& ) {
	mView->modePool()->push( MODE_COMPLETION );
}
void YZModeInsert::commandCompletionNext( YZView* mView, const QString& ) {
	mView->modePool()->push( MODE_COMPLETION );
	YZModeCompletion* c = static_cast<YZModeCompletion*>( mView->modePool()->current() );
	c->execCommand(mView, "<CTRL>n");
}
void YZModeInsert::commandCompletionPrevious( YZView* mView, const QString& ) {
	mView->modePool()->push( MODE_COMPLETION );
	YZModeCompletion* c = static_cast<YZModeCompletion*>( mView->modePool()->current() );
	c->execCommand(mView, "<CTRL>p");
}
void YZModeInsert::commandDown( YZView* mView, const QString& ) {
	mView->moveDown();
}
void YZModeInsert::commandUp( YZView* mView, const QString& ) {
	mView->moveUp();
}
void YZModeInsert::commandLeft( YZView* mView, const QString& ) {
	mView->moveLeft();
}
void YZModeInsert::commandRight( YZView* mView, const QString& ) {
	mView->moveRight();
}
void YZModeInsert::commandPageDown( YZView* mView, const QString& ) {
	int line = mView->getCurrentTop() + mView->getLinesVisible();

	if (mView->getLocalBooleanOption("wrap")) {
		YZViewCursor temp = mView->viewCursor();
		mView->gotodxdy( &temp, mView->getDrawCurrentLeft(),
				mView->getDrawCurrentTop() + mView->getLinesVisible() );

		line = temp.bufferY();
	}

	// don't scroll below the last line of the buffer
	if (line > mView->myBuffer()->lineCount())
		line = mView->myBuffer()->lineCount();

	// scroll the view one screen down, and move the cursor to the first nonblank on
	// the line it was moved to, like vim does.
	if (line != mView->getCurrentTop()) {
		mView->alignViewBufferVertically( line );
		mView->moveToFirstNonBlankOfLine();
	}
}
void YZModeInsert::commandPageUp( YZView* mView, const QString& ) {
	int line = mView->getCurrentTop() - mView->getLinesVisible();

	if (line < 0)
		line = 0;

	if (line != (int)mView->getCurrentTop()) {
		mView->alignViewBufferVertically( line );
		mView->moveToFirstNonBlankOfLine();
	}
}
void YZModeInsert::commandBackspace( YZView* mView, const QString& ) {
	YZCursor cur = mView->getBufferCursor();
	YZBuffer* mBuffer = mView->myBuffer();
	if ( cur.x() == 0 && cur.y() > 0 && mView->getLocalStringOption( "backspace" ).contains( "eol" ) ) {
		mBuffer->action()->mergeNextLine( mView, cur.y() - 1 );
		//mBuffer->action()->deleteChar( mView, *mView->getBufferCursor(), 1 ); see bug #158
	} else if ( cur.x() > 0 ) {
		mBuffer->action()->deleteChar( mView, cur.x() - 1, cur.y(), 1 );
	}
}
void YZModeInsert::commandDel( YZView* mView, const QString& ) {
	YZCursor cur = mView->getBufferCursor();
	YZBuffer* mBuffer = mView->myBuffer();
	if ( cur.x() == mBuffer->textline( cur.y() ).length() 
			&& mView->getLocalStringOption( "backspace" ).contains( "eol" ) ) {
		mBuffer->action()->mergeNextLine( mView, cur.y(), false );
	} else {
		mBuffer->action()->deleteChar( mView, cur, 1 );
	}
}
void YZModeInsert::commandEnter( YZView* mView, const QString& ) {
	YZCursor cur = mView->getBufferCursor();
	YZBuffer* mBuffer = mView->myBuffer();
	if ( mView->getLocalBooleanOption("cindent") ) {
		mView->indent();
	} else {
		mBuffer->action()->insertNewLine( mView, cur );
		QStringList results = YZSession::self()->eventCall("INDENT_ON_ENTER", mView);
		if (results.count() > 0 ) {
			if (results[0].length()!=0) {
				mBuffer->action()->replaceLine( mView, cur.y()+1, results[0] + mBuffer->textline( cur.y()+1 ).trimmed() );
				mView->gotoxy(results[0].length(),cur.y()+1);
			}
		}
	}
	mView->updateStickyCol();
}
cmd_state YZModeInsert::commandDefault( YZView* mView, const QString& key ) {
	mView->myBuffer()->action()->insertChar( mView, mView->getBufferCursor(), key );
	if ( mView->getLocalBooleanOption( "cindent" ) && key == "}" )
		mView->reindent( mView->getBufferCursor().x() - 1, mView->getBufferCursor().y() );
	return CMD_OK;
}

void YZModeInsert::imBegin( YZView* ) {
	m_imPreedit = "";
}
void YZModeInsert::imCompose( YZView* mView, const QString& entry ) {
	if ( !m_imPreedit.isEmpty() ) { // replace current one
		YZCursor pos = mView->getBufferCursor();
		int len = m_imPreedit.length();
		if ( pos.x() >= len )
			pos.setX( pos.x() - len );
		else
			pos.setX( 0 );
		mView->myBuffer()->action()->replaceText( mView, pos, len, entry );
	} else {
		mView->sendKey( entry );
	}
	m_imPreedit = entry;
}
void YZModeInsert::imEnd( YZView* mView, const QString& entry ) {
	imCompose( mView, entry );
	m_imPreedit = "";
}


/**
 * YZModeReplace
 */

YZModeReplace::YZModeReplace() : YZModeInsert() {
	mType = MODE_REPLACE;
	mString = _("[ Replace ]");
}

void YZModeReplace::commandInsert( YZView* mView, const QString& ) {
	mView->modePool()->change( MODE_INSERT, false );
}
void YZModeReplace::commandBackspace( YZView* mView, const QString& key ) {
	commandLeft( mView, key );
}

cmd_state YZModeReplace::commandDefault( YZView* mView, const QString& key ) {
	mView->myBuffer()->action()->replaceChar( mView, mView->getBufferCursor(), key );
	return CMD_OK;
}

