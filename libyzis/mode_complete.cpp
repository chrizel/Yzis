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
 * $Id: /yzis/trunk/libyzis/mode_insert.cpp 167 2005-06-17T02:15:33.665919Z morbuz  $
 */

#include "portability.h"
#include "mode_complete.h"

#include "buffer.h"
#include "session.h"
#include "action.h"
#include "viewcursor.h"
#include "mode_command.h"
#include "tags_interface.h"
#include "internal_options.h"

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

void YZModeCompletion::leave( YZView* /*view*/ ) {
	mPrefix = QString::null;
}

bool YZModeCompletion::initCompletion( YZView* view, bool forward ) {
	YZBuffer* buffer = view->myBuffer();
	YZMotionArgs arg(view, 1);
	YZCursor cur = *view->getBufferCursor();
	QString line = buffer->textline(cur.y());
	
	//we cant complete from col 0, neither if the line is empty, neither if the word does not end with a letter or number ;)
	if (cur.x() == 0 || line.isEmpty() || !QChar(line.at(cur.x()-1)).isLetterOrNumber()) {
		yzDebug() << "Abort completion" << endl;
		view->modePool()->pop();
		return false;
	}
	
	mCompletionStart = YZSession::me->getCommandPool()->moveWordBackward( arg );
	YZCursor stop( cur.x()-1, cur.y() );
	yzDebug() << "Start : " << mCompletionStart << ", End:" << stop << endl;
	QStringList list = buffer->getText(mCompletionStart, stop);
	yzDebug() << "Completing word : " << list[0] << endl;
	
	// if there's nothing to complete, abort
	if (list[0].isEmpty()) {
		yzDebug() << "Abort completion" << endl;
		view->modePool()->pop();
		return false;
	}
	
	mPrefix = list[0];
	mCompletionEnd = cur;
	
	mProposedCompletions = mPrefix;
	mCurrentProposal = 0;
	mLastMatch = "";
	
	yzDebug() << "COMPLETION: mPrefix: " << mPrefix << endl;
	
	QStringList completeOption = YZSession::me->getOptions()->readListOption("complete", QStringList(".") << "w" << "b" << "u" << "t" << "i");
	
	for ( unsigned int i = 0; i < completeOption.size(); ++i ) {
		QString option = completeOption[ i ];

		if ( option == "."  ) {
			completeFromCurrentBuffer( mCompletionStart, forward, mProposedCompletions );
		} else if ( option == "w" || option == "u" || option == "U" || option == "b" ) { 
			// according to the VIM docs, these are separate, but I'm not distinguishing between
			// them yet
			completeFromOtherBuffers( buffer, mProposedCompletions );
		} else if ( option == "t" || option == "]" ) {
			// these options are the same according to the VIM docs
			completeFromTags( mProposedCompletions );
		}
	}

	return true;
}

void YZModeCompletion::doComplete( YZView* view, bool forward ) {
	// move iterator
	// first do bounds check.  Do wrap around.
	if ( forward && mCurrentProposal == mProposedCompletions.size() - 1 ) {
		mCurrentProposal = 0;
	} else if ( !forward && mCurrentProposal == 0 ) {
		mCurrentProposal = mProposedCompletions.size() - 1;
	} else if ( forward ) {
		++mCurrentProposal;
	} else {
		--mCurrentProposal;
	}
	
	// replace text
	QString proposal = mProposedCompletions[ mCurrentProposal ];
	YZAction *action = view->myBuffer()->action();
	YZCursor currentCursor = *view->getBufferCursor();
	action->replaceText( view, mCompletionStart, currentCursor.x() - mCompletionStart.x(), proposal );
	view->gotoxy( mCompletionStart.x() + proposal.length(), currentCursor.y() );
	
	// display match number in the display bar
	QString msg( _("Match %1") );
	msg = msg.arg( mCurrentProposal );
	if ( mCurrentProposal == 0 ) {
		msg = _("Back at original");
	}
	
	view->displayInfo( msg );
}

cmd_state YZModeCompletion::execCommand( YZView* view, const QString& _key ) {
	YZCursor cur = *view->getBufferCursor();
	bool initOK = true;
	
	// if we're to cycle through the potential matches, do the cycling
	if ( _key == "<CTRL>n" || _key == "<CTRL>p" ) {
		// check if we need to generated the matches
		if ( mPrefix.isEmpty() ) {
			mLastKey = _key;
			bool forward = true; // this is different than mForward and indicates the direction of user search
			if ( _key == "<CTRL>p" ) {
				forward = false;
			}
			
			initOK = initCompletion( view, forward );
			// we want to do forwards through the search list.
			mForward = true;
		}
		
		// now get the next completion
		if ( initOK ) {
			// we want to reverse our direction through the list
			// if we're going in a different direction now
			// keep track of this by storing the last key hit
			if ( mLastKey != _key ) {
				mForward = !mForward;
			}
			
			doComplete( view, mForward );
			
			mLastKey = _key;
		}
	} else if ( _key == "<CTRL>x" ) {
		yzDebug() << "Skip CTRLx in completion mode" << endl;
		return CMD_OK;
	} else {
		view->modePool()->pop();
		view->modePool()->replayKey();
		return CMD_OK;

	}
	return CMD_ERROR;
}

void YZModeCompletion::completeFromBuffer( YZBuffer *buffer, QStringList &proposed, bool elimDups /*=true*/, YZList<YZCursor> *cursors /*=NULL*/ )
{
	// Guardian for empty buffers
	if ( buffer->isEmpty() ) {
		return;
	}
	
	unsigned int matchedLength;
	bool found;
	YZCursor matchCursor;
	YZCursor nextCursor;
	YZCursor endCursor;
	YZAction *action = buffer->action();
	const QString pattern = "\\b" + mPrefix + "\\w*";
	const YZCursor bufbegin(0, 0);
	const YZCursor bufend(0, buffer->lineCount() + 1);
	
	yzDebug() << "COMPLETION: pattern: " << pattern << endl;
	
	// set the initial search start
	nextCursor = bufbegin;
	endCursor = bufend;
	
	// loop through the buffer looking for matches
	// at each match we set nextCursor to the place we would have scanned next if
	// we didn't find the match.  This lets us continue through the entire buffer
	do {
		// search from the end of the prefix to the end of the buffer
		matchCursor = action->search(buffer, pattern, nextCursor, endCursor, &matchedLength, &found);
		nextCursor = YZCursor( matchCursor.x() + matchedLength, matchCursor.y() );
		
		if ( found ) {
			QString possible = buffer->getWordAt( matchCursor );
			// add to the proposed 
			// the contains check is to ensure we don't add duplicates
			// This is O(n^2), but hopefully it won't kill us, since the
			// sets are likely small
			// but only do the contains check if we're to eliminate duplicates
			if ( !elimDups || elimDups && proposed.contains( possible ) == 0 ) {
				proposed.push_back( possible );
				if ( cursors ) {
					cursors->push_back( matchCursor );
				}
			}
		}
	} while( found );

	yzDebug() << "COMPLETION: Found " << proposed.size() << " matches" << endl;
}

void YZModeCompletion::completeFromOtherBuffers( YZBuffer *skip, QStringList &proposed )
{
	// for each buffer, call completeFromBuffer
	YZBufferList buffers = YZSession::me->buffers();
	for ( YZBufferList::iterator itr = buffers.begin(); itr != buffers.end(); ++itr ) {
		YZBuffer *cur = *itr;

		yzDebug() << "COMPLETION: Inspecting another buffer" << endl;
		
		// don't descend into the buffer we're told to skip
		// this is done so that one buffer (probably the current)
		// has priority in the list
		if ( cur != skip ) {
			completeFromBuffer( cur, proposed );
		}
	}
}

void YZModeCompletion::completeFromIncludes( QStringList &/*proposed*/ )
{
}

void YZModeCompletion::completeFromTags( QStringList &proposed )
{
	QStringList tags;
	tagStartsWith( mPrefix, tags );
	
	for ( unsigned int i = 0; i < tags.size(); ++i ) {
		if ( proposed.contains( tags[i] ) == 0 ) {
			proposed.push_back( tags[i] );
		}
	}
}

void YZModeCompletion::completeFromDictionary( QStringList &/*proposed*/ )
{
}

void YZModeCompletion::completeFromFileNames( QStringList &/*proposed*/ )
{
}

void YZModeCompletion::completeFromCurrentBuffer( const YZCursor &cursor, bool forward, QStringList &proposed )
{
	YZBuffer *buffer = YZSession::me->currentView()->myBuffer();
	
	QStringList matches;
	YZList<YZCursor> cursorlist;
	
	completeFromBuffer( buffer, matches, false, &cursorlist );
	
	// ASSERTION: when scanning the current buffer, we must
	// 		find the search pattern at least once, since
	//			we'll find it where we typed
	//			Therefore: cursor must appear exactly once in
	//			the list cursors!
	YZASSERT_MSG( cursorlist.contains( cursor ) == 1, "Current cursor not found in list of matched cursors" );
	
	// use the above fact to locate where in the cursors list
	// we should start scanning from
	unsigned int startidx = cursorlist.findIndex( cursor );

	int delta = forward ? 1 : -1; // direction to search through the list
	
	// now add all elements in matches to proposed, starting with the element
	// following startidx and wrapping all the way around
	// we loop until we've added matches.size() - 1 elements, because
	// we don't want to add the element at the cursor to the list
	for ( unsigned int count = 0, i = ( startidx + delta ) % matches.size();
			count < matches.size() - 1; 
			++count, i = (i + delta) % matches.size() ) {
		if ( !proposed.contains( matches[ i ] ) ) {
			proposed.push_back( matches[ i ] );
		}
	}
}
