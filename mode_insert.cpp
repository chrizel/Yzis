/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2005 Erlend Hamberg <ehamberg@online.no>
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

#include "mode_insert.h"

#include "buffer.h"
#include "session.h"
#include "action.h"
#include "viewcursor.h"
#include "mode_command.h"
#include "libintl.h"

YZModeInsert::YZModeInsert() : YZMode() {
	mType = YZMode::MODE_INSERT;
	mString = _( "[ Insert ]" );
	mEditMode = true;
	mIM = true;
	mMapMode = insert;
}
void YZModeInsert::leave( YZView* mView ) {
	if ( mView->getBufferCursor()->x() > 0 )
		mView->moveLeft();
}

void YZModeInsert::initModifierKeys() {
	mModifierKeys << "<CTRL>e" << "<CTRL>n" << "<CTRL>p" << "<CTRL>x" << "<CTRL>y" << "<ALT>:" << "<ALT>v";
}
/*
 * if you add a command which use modifiers keys, add it in initModifierKeys too
 */
cmd_state YZModeInsert::execCommand( YZView* mView, const QString& _key ) {
	QString key = _key;
	cmd_state ret = CMD_OK;
	     if ( key == "<HOME>" ) commandHome( mView, key );
	else if ( key == "<END>" ) commandEnd( mView, key );
	else if ( key == "<ESC>" ) commandEscape( mView, key );
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
		if ( key == "<TAB>" ) key = "\t";
		/* if ( key.startsWith("<CTRL>") ) // XXX no sense
			ret = YZSession::me->getCommandPool()->execCommand(mView, key);
		else*/
		ret = commandDefault( mView, key );
		QStringList ikeys = mView->myBuffer()->getLocalListOption("indentkeys");
		if ( ikeys.contains(key) )
			YZSession::events->exec("INDENT_ON_KEY", mView);
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
	unsigned int line = mView->getCurrentTop() + mView->getLinesVisible();

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
	YZCursor cur = *mView->getBufferCursor();
	YZBuffer* mBuffer = mView->myBuffer();
	if ( cur.x() == 0 && cur.y() > 0 && mView->getLocalStringOption( "backspace" ).contains( "eol" ) ) {
		mBuffer->action()->mergeNextLine( mView, cur.y() - 1 );
		//mBuffer->action()->deleteChar( mView, *mView->getBufferCursor(), 1 ); see bug #158
	} else if ( cur.x() > 0 ) {
		mBuffer->action()->deleteChar( mView, cur.x() - 1, cur.y(), 1 );
	}
}
void YZModeInsert::commandDel( YZView* mView, const QString& ) {
	YZCursor cur = *mView->getBufferCursor();
	YZBuffer* mBuffer = mView->myBuffer();
	if ( cur.x() == mBuffer->textline( cur.y() ).length() && mView->getLocalStringOption( "backspace" ).contains( "eol" ) ) {
		mBuffer->action()->mergeNextLine( mView, cur.y(), false );
	} else {
		mBuffer->action()->deleteChar( mView, cur, 1 );
	}
}
void YZModeInsert::commandEnter( YZView* mView, const QString& ) {
	YZCursor cur = *mView->getBufferCursor();
	YZBuffer* mBuffer = mView->myBuffer();
	if ( mView->getLocalBooleanOption("cindent") ) {
		mView->indent();
	} else {
		mBuffer->action()->insertNewLine( mView, cur );
		QStringList results = YZSession::events->exec("INDENT_ON_ENTER", mView);
		if (results.count() > 0 ) {
			if (results[0].length()!=0) {
#if QT_VERSION < 0x040000
				mBuffer->action()->replaceLine( mView, cur.y()+1, results[0] + mBuffer->textline( cur.y()+1 ).stripWhiteSpace() );
#else
				mBuffer->action()->replaceLine( mView, cur.y()+1, results[0] + mBuffer->textline( cur.y()+1 ).trimmed() );
#endif
				mView->gotoxy(results[0].length(),cur.y()+1);
			}
		}
	}
	mView->updateStickyCol();
}
cmd_state YZModeInsert::commandDefault( YZView* mView, const QString& key ) {
	mView->myBuffer()->action()->insertChar( mView, mView->getBufferCursor(), key );
	if ( mView->getLocalBooleanOption( "cindent" ) && key == "}" )
		mView->reindent( mView->getBufferCursor()->x() - 1, mView->getBufferCursor()->y() );
	return CMD_OK;
}

void YZModeInsert::imBegin( YZView* ) {
	m_imPreedit = "";
}
void YZModeInsert::imCompose( YZView* mView, const QString& entry ) {
	if ( !m_imPreedit.isEmpty() ) { // replace current one
		YZCursor pos = *mView->getBufferCursor();
		unsigned int len = m_imPreedit.length();
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



/**
 * YZModeCompletion
 */

YZModeCompletion::YZModeCompletion() : YZMode() {
	mType = MODE_COMPLETION;
	mString = _("{ Completion }");
	mEditMode = true;
	mMapMode = insert;
}
YZModeCompletion::~YZModeCompletion() {
}

void YZModeCompletion::leave( YZView* mView ) {
	mView->m_word2Complete = QString::null;
}

bool YZModeCompletion::initCompletion( YZView* mView ) {
	YZCursor* m_completionStart = mView->m_completionStart;
	YZCursor* m_completionCursor = mView->m_completionCursor;
	YZBuffer* mBuffer = mView->myBuffer();
	YZMotionArgs arg(mView, 1);
	YZCursor cur = mView->getBufferCursor();
	QString line = mBuffer->textline(cur.y());
	//we cant complete from col 0, neither if the line is empty, neither if the word does not end with a letter or number ;)
	if (cur.x() == 0 || line.isEmpty() || !QChar(line.at(cur.x()-1)).isLetterOrNumber()) {
		yzDebug() << "Abort completion" << endl;
		mView->modePool()->pop();
		return false;
	}
	YZCursor begin = YZSession::me->getCommandPool()->moveWordBackward( arg );
	m_completionStart->setCursor(begin);
	YZCursor stop( cur.x()-1, cur.y() );
	yzDebug() << "Start : " << begin << ", End:" << stop << endl;
	QStringList list = mBuffer->getText(begin, stop);
	yzDebug() << "Completing word : " << list[0] << endl;
	//record current begin-of-word-to-complete
	if (list[0].isEmpty()) {
		mView->modePool()->pop();
		return false;
	}
	mView->m_word2Complete = list[0];
	m_completionCursor->setCursor( cur );
	mView->m_oldProposals.clear();
	mView->m_lastCompletionDir = true;
	mView->m_lastMatch = QString::null;
	return true;
}

QString YZModeCompletion::doComplete( YZView* mView, bool forward ) {
	YZCursor result;
	unsigned int matchedLength=0;
	bool found=false;
	QString list="";

	YZBuffer* mBuffer = mView->myBuffer();
	QString m_word2Complete = mView->m_word2Complete;
	YZCursor* m_completionCursor = mView->m_completionCursor;
	YZCursor cur = *mView->getBufferCursor();
	
	if ( mView->m_lastCompletionDir != forward) {
		mView->m_oldProposals.clear();
		mView->m_lastCompletionDir = forward;
	}
	
	do {
		if (forward) {
			result = mBuffer->action()->search(mView, "\\b" + m_word2Complete + "\\w*", *m_completionCursor, YZCursor(0, mBuffer->lineCount()+1), false, &matchedLength, &found);
		} else {
			if ( *m_completionCursor == cur )
				m_completionCursor->setX( cur.x() - m_word2Complete.length() );
			result = mBuffer->action()->search(mView, "\\b" + m_word2Complete + "\\w*", *m_completionCursor, YZCursor(0, 0), true, &matchedLength, &found);
		}
		if (found) {
			YZCursor end ( result.x()+matchedLength-1, result.y());
			list = mBuffer->getText(result, end)[0];
//			yzDebug() << "Got testing match : " << list << " at " << result << " to " << end << endl;
			m_completionCursor->setCursor(result);
			if (forward) {
				if ( m_completionCursor->x() < mBuffer->textline(m_completionCursor->y()).length() )
					m_completionCursor->setX(m_completionCursor->x()+1);
				else {
					m_completionCursor->setY(m_completionCursor->y()+1);
					m_completionCursor->setX(0);
				}
			}
		}
	} while ( found && ( list == mView->m_lastMatch || mView->m_oldProposals.contains(list)) );

	//found something ?
	if ( found )  {
		yzDebug() << "Match : " << list << endl;
		mView->m_lastMatch = list;
		mView->m_oldProposals << list;
		return list;
	}
	//no more result clear the list if we want to go for another round
	mView->m_oldProposals.clear();
	return QString::null;
}

cmd_state YZModeCompletion::execCommand( YZView* mView, const QString& _key ) {
	YZBuffer* mBuffer = mView->myBuffer();
	YZCursor* m_completionStart = mView->m_completionStart;
	YZCursor cur = *mView->getBufferCursor();
	bool initOK = true;

	if ( _key == "<CTRL>p" ) {
		if (mView->m_word2Complete.isEmpty())
			initOK = initCompletion( mView );
		if (initOK) {
			QString result = doComplete( mView, false );
			if (!result.isNull()) {
				mBuffer->action()->replaceText(mView, *m_completionStart, cur.x()-m_completionStart->x(), result);
				mView->gotoxy(m_completionStart->x()+result.length(),cur.y());
			}
		}
		return CMD_OK;

	} else if ( _key == "<CTRL>n" ) {
		if (mView->m_word2Complete.isEmpty())
			initOK = initCompletion( mView );
		if (initOK) {
			QString result = doComplete( mView, true );
			if (!result.isNull()) {
				mBuffer->action()->replaceText(mView, *m_completionStart, cur.x()-m_completionStart->x(), result);
				mView->gotoxy(m_completionStart->x()+result.length(),cur.y());
			}
		}
		return CMD_OK;

	} else if ( _key == "<CTRL>x" ) {
		yzDebug() << "Skip CTRLx in completion mode" << endl;
		return CMD_OK;

	} else if ( _key == "<ESC>" ) {
		mBuffer->action()->replaceText(mView, *m_completionStart, cur.x()-m_completionStart->x(), mView->m_word2Complete);
		mView->gotoxy(m_completionStart->x()+mView->m_word2Complete.length(),cur.y());
		mView->modePool()->pop( YZMode::MODE_COMMAND );
		return CMD_OK;

	} else {
		mView->modePool()->pop();
		mView->modePool()->replayKey();
		return CMD_OK;

	}
	return CMD_ERROR;
}


