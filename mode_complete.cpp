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
	YZCursor cur = view->getBufferCursor();
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
	
	if ( forward ) {
		completeFromBuffer( buffer, forward, mCompletionEnd, YZCursor(0, buffer->lineCount() + 1), true, mProposedCompletions );
	} else {
		completeFromBuffer( buffer, forward, mCompletionStart, YZCursor(0, 0), true, mProposedCompletions );
	}
	
	completeFromOtherBuffers( buffer, mProposedCompletions );
	completeFromTags( mProposedCompletions );

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
	YZCursor currentCursor = view->getBufferCursor();
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

void YZModeCompletion::completeFromBuffer( YZBuffer *buffer, bool forward, const YZCursor &initBegin, const YZCursor &initEnd, bool doWrap, QStringList &proposed )
{
	unsigned int matchedLength;
	bool found;
	YZCursor matchCursor;
	YZCursor nextCursor;
	YZCursor endCursor;
	YZAction *action = buffer->action();
	const QString pattern = "\\b" + mPrefix + "\\w*";
	const YZCursor bufbegin(0, 0);
	const YZCursor bufend(0, buffer->lineCount() + 1);
	bool wrapped = !doWrap;
	
	yzDebug() << "COMPLETION: pattern: " << pattern << endl;
	
	// set the initial search start
	nextCursor = initBegin;
	endCursor = initEnd;
	
	// loop through the buffer looking for matches
	// at each match we set nextCursor to the place we would have scanned next if
	// we didn't find the match.  This lets us continue through the entire buffer
	do {
		if ( forward ) {
			// search from the end of the prefix to the end of the buffer
			matchCursor = action->search(buffer, pattern, nextCursor, endCursor, &matchedLength, &found);
			nextCursor = YZCursor( matchCursor.x() + matchedLength, matchCursor.y() );
		} else {
			// search from the beginning of the prefix to the beginning of the buffer
			matchCursor = action->search(buffer, pattern, nextCursor, endCursor, &matchedLength, &found);
			nextCursor = matchCursor;
		}
		
		// check for wrap around
		if ( !found && !wrapped ) {
			wrapped = true;
			found = true; // must set this to true to avoid leaving the loop IMPORTANT: nothing below here relies on found!
			
			// continue search from beginning of the buffer until
			// just before the completion started
			if ( forward ) {
				nextCursor = bufbegin;
				endCursor = mCompletionStart;
			} 
			// continue search from the end of the buffer until
			// just after the completion ends
			else {
				nextCursor = bufend;
				endCursor = mCompletionEnd;
			}
		} 
		// no wrap around, possible match
		else if ( found ) {
			QString possible = buffer->getWordAt( matchCursor );
			// add to the proposed 
			// the contains check is to ensure we don't add duplicates
			// This is O(n^2), but hopefully it won't kill us, since the
			// sets are likely small
			if ( proposed.contains( possible ) == 0 ) {
				yzDebug() << "COMPLETION: Possible match: " << possible << endl;
				proposed.push_back( possible );
			}
		}
	} while( found || !wrapped ); // don't quit until we've wrapped around and then not found any matches

	yzDebug() << "COMPLETION: Found " << proposed.size() << " matches" << endl;
}

void YZModeCompletion::completeFromOtherBuffers( YZBuffer *skip, QStringList &proposed )
{
	// for each buffer, call completeFromBuffer
	YZBufferMap buffers = YZSession::me->buffers();
	for ( YZBufferMap::iterator itr = buffers.begin(); itr != buffers.end(); ++itr ) {
		YZBuffer *cur = *itr;

		yzDebug() << "COMPLETION: Inspecting another buffer" << endl;
		
		// don't descend into the buffer we're told to skip
		// this is done so that one buffer (probably the current)
		// has priority in the list
		if ( cur != skip ) {
			// always search forward
			completeFromBuffer( cur, true, YZCursor(0, 0), YZCursor(0, cur->lineCount() + 1), false, proposed );
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
