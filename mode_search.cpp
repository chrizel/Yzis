/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
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
#include "mode_search.h"

#include "debug.h"

#include "action.h"
#include "buffer.h"
#include "search.h"
#include "selection.h"
#include "session.h"

YZModeSearch::YZModeSearch() : YZMode() {
	mType = YZMode::MODE_SEARCH;
	mString = _( "[ Search ]" );
	mMapMode = cmdline;
}
YZModeSearch::~YZModeSearch() {
}
void YZModeSearch::enter( YZView* mView ) {
	YZSession::me->setFocusCommandLine();
	mView->setCommandLineText( "" );
	mView->mSearchBegin->setCursor( mView->getBufferCursor() );
}
void YZModeSearch::leave( YZView* mView ) {
	mView->setCommandLineText( "" );
	YZSession::me->setFocusMainWindow();
}

YZCursor YZModeSearch::replaySearch( YZView* mView, bool* found ) {
	return YZSession::me->search()->replayForward( mView, found );
}
YZCursor YZModeSearch::search( YZView* mView, const QString& s, bool* found ) {
	return YZSession::me->search()->forward( mView, s, found );
}
YZCursor YZModeSearch::search( YZView* mView, const QString& s, const YZCursor& begin, unsigned int* matchlength, bool* found ) {
	YZCursor end( 0, mView->myBuffer()->lineCount() - 1 );
	end.setX( mView->myBuffer()->textline( end.y() ).length() );
	return mView->myBuffer()->action()->search( mView, s, begin, end, false, matchlength, found );
}

void YZModeSearch::initModifierKeys() {
	mModifierKeys << "<ALT>:";
}
cmd_state YZModeSearch::execCommand( YZView* mView, const QString& _key ) {
	QString key = _key;
	YZCursor* mSearchBegin = mView->mSearchBegin;
	YZCursor* incSearchResult = mView->incSearchResult;
	YZSelection* searchSelection = mView->getSelectionPool()->search();

	if ( key == "<ENTER>" ) {
		QString what = mView->getCommandLineText();
		yzDebug() << "Current search : " << what;

		bool found = false;
		YZCursor pos;
		if ( what.isEmpty() ) {
			pos = replaySearch( mView, &found );
		} else {
			mView->mSearchHistory[ mView->mCurrentSearchItem++ ] = what;
			pos = search( mView, what, &found );
			if ( mView->getLocalBooleanOption( "incsearch" ) && mView->incSearchFound ) {
				pos = *mView->incSearchResult;
				mView->incSearchFound = false;
			}
		}
		if ( found ) {
			mView->gotoxy( pos.x(), pos.y() );
		} else {
			mView->displayInfo(_("No match"));
		}
		mView->modePool()->pop();
		return CMD_OK;
	} else if ( key == "<DOWN>" ) {
		if (mView->mSearchHistory[mView->mCurrentSearchItem].isEmpty())
			return CMD_OK;
		mView->mCurrentSearchItem++;
		mView->setCommandLineText( mView->mSearchHistory[mView->mCurrentSearchItem] );
		return CMD_OK;
	} else if ( key == "<LEFT>" || key == "<RIGHT>" ) {
		return CMD_OK;
	} else if ( key == "<UP>" ) {
		if(mView->mCurrentSearchItem == 0)
			return CMD_OK;
		mView->mCurrentSearchItem--;
		mView->setCommandLineText( mView->mSearchHistory[mView->mCurrentSearchItem] );
		return CMD_OK;
	} else if ( key == "<ALT>:" ) {
		mView->modePool()->change( MODE_EX );
		return CMD_OK;
	} else if ( key == "<ESC>" ) {
		if ( mView->getLocalBooleanOption( "incsearch" ) ) {
			mView->gotoxy(mSearchBegin->x(), mSearchBegin->y());
			mView->setPaintAutoCommit( false );
			mView->incSearchFound = false;
			mView->sendPaintEvent( searchSelection->map() );
			searchSelection->clear();
			mView->commitPaintEvent();
		}
		mView->modePool()->pop();
		return CMD_OK;
	} else if ( key == "<BS>" ) {
		QString back = mView->getCommandLineText();
		mView->setCommandLineText(back.remove(back.length() - 1, 1));
	} else {
		mView->setCommandLineText( mView->getCommandLineText() + key );
	}

	if ( mView->getLocalBooleanOption("incsearch") ) {
		mView->setPaintAutoCommit( false );
		unsigned int matchlength;
		incSearchResult->setCursor( search(mView, mView->getCommandLineText(), *mSearchBegin, &matchlength, &(mView->incSearchFound)) );
		if ( mView->incSearchFound ) {
			if ( mView->getLocalBooleanOption("hlsearch") ) {
				YZCursor endResult( incSearchResult );
				endResult.setX( endResult.x() + matchlength - 1 );
				searchSelection->addInterval( YZInterval(*incSearchResult, endResult) );
				mView->sendPaintEvent( searchSelection->map() );
			}
			mView->gotoxyAndStick( incSearchResult );
		} else {
			mView->gotoxy( mSearchBegin->x(), mSearchBegin->y() );
			mView->sendPaintEvent( searchSelection->map() );
			searchSelection->clear();
		}
		mView->commitPaintEvent();
	}
	return CMD_OK;
}


YZModeSearchBackward::YZModeSearchBackward() : YZModeSearch() {
	mType = YZMode::MODE_SEARCH_BACKWARD;
	mString = _( "[ Search backward ]" );
}
YZModeSearchBackward::~YZModeSearchBackward() {
}

YZCursor YZModeSearchBackward::replaySearch( YZView* mView, bool * found ) {
	return YZSession::me->search()->replayBackward( mView, found );
}
YZCursor YZModeSearchBackward::search( YZView* mView, const QString& s, bool* found ) {
	YZCursor buffer = *mView->getBufferCursor();
	mView->gotoxy( buffer.x() + 1, buffer.y(), false );
	return YZSession::me->search()->backward( mView, s, found );
}
YZCursor YZModeSearchBackward::search( YZView* mView, const QString& s, const YZCursor& begin, unsigned int* matchlength, bool* found ) {
	YZCursor end( 0, 0 );
	return mView->myBuffer()->action()->search( mView, s, begin, end, true, matchlength, found );
}



