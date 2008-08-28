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

#include "mode_complete.h"
#include "mode_command.h"
#include "mode_pool.h"
#include "buffer.h"
#include "debug.h"
#include "session.h"
#include "action.h"
#include "view.h"
#include "tags_interface.h"
#include "internal_options.h"

/**
 * YModeCompletion
 */

#define dbg()    yzDebug("YModeCompletion")
#define err()    yzError("YModeCompletion")

using namespace yzis;

YModeCompletion::YModeCompletion() : YMode()
{
    mType = ModeCompletion;
    mString = _("{ Completion }");
    mMapMode = MapInsert;
    mIsEditMode = true;
    mIsCmdLineMode = false;
    mIsSelMode = false;
}
YModeCompletion::~YModeCompletion()
{}

void YModeCompletion::leave( YView* /*view*/ )
{
    mPrefix = QString();
}

bool YModeCompletion::initCompletion( YView* view, bool forward )
{
    CmdState state;
    YBuffer* buffer = view->myBuffer();
    YMotionArgs arg(view, 1);
    YCursor cur = view->getBufferCursor();
    QString line = buffer->textline(cur.y());

    //we cant complete from col 0, neither if the line is empty, neither if the word does not end with a letter or number ;)
    if (cur.x() == 0 || line.isEmpty() || !QChar(line.at(cur.x() - 1)).isLetterOrNumber()) {
        dbg() << "Abort completion" << endl;
        view->modePool()->pop();
        return false;
    }

    mCompletionStart = YSession::self()->getCommandPool()->moveWordBackward( arg, &state );
    YCursor stop( cur.x() - 1, cur.y() );
    dbg() << "Start : " << mCompletionStart << ", End:" << stop << endl;
	YRawData list = buffer->dataRegion(YInterval(mCompletionStart, stop));
    dbg() << "Completing word : " << list[0] << endl;

    // if there's nothing to complete, abort
    if (list[0].isEmpty()) {
        dbg() << "Abort completion" << endl;
        view->modePool()->pop();
        return false;
    }

    mPrefix = list[0];
    mCompletionEnd = cur;

    mProposedCompletions = QStringList( mPrefix );
    mCurrentProposal = 0;
    mLastMatch = "";

    dbg() << "COMPLETION: mPrefix: " << mPrefix << endl;

    QStringList completeOption = YSession::self()->getOptions()->readListOption("complete", QStringList(".") << "w" << "b" << "u" << "t" << "i");

    for ( int i = 0; i < completeOption.size(); ++i ) {
        QString option = completeOption[ i ];

        if ( option == "." ) {
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

void YModeCompletion::doComplete( YView* view, bool forward )
{
    // move iterator
    // first do bounds check.  Do wrap around.
    if ( forward && (int)mCurrentProposal == mProposedCompletions.size() - 1 ) {
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
    YCursor currentCursor = view->getBufferCursor();
    action->replaceText( view, mCompletionStart, currentCursor.x() - mCompletionStart.x(), proposal );
    view->gotoxy( mCompletionStart.x() + proposal.length(), currentCursor.y() );

    // display match number in the display bar
    QString msg( _("Match %1 of %2") );
    msg = msg.arg( mCurrentProposal ).arg( mProposedCompletions.size() - 1 );
    if ( mCurrentProposal == 0 ) {
        msg = _("Back at original");
    }

    view->displayInfo(msg);
}

CmdState YModeCompletion::execCommand( YView* view, const YKeySequence &keys, 
                                       YKeySequence::const_iterator &parsePos )
{
    bool initOK = true;
    YKey _key = *parsePos;

    // if we're to cycle through the potential matches, do the cycling
    if ( _key == YKey(Qt::Key_N, Qt::ControlModifier) || _key == YKey(Qt::Key_P, Qt::ControlModifier) ) {
        // check if we need to generated the matches
        if ( mPrefix.isEmpty() ) {
            mLastKey = _key;
            bool forward = true; // this is different than mForward and indicates the direction of user search
            if ( _key == YKey(Qt::Key_P, Qt::ControlModifier) ) {
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
        // Have used key, so advance pointer
        ++parsePos;
    } else if ( _key == YKey(Qt::Key_X, Qt::ControlModifier) ) {
        dbg() << "Skip CTRLx in completion mode" << endl;
        ++parsePos;
        return CmdOk;
    } else {
        view->modePool()->pop();
        view->modePool()->replayKey();
        return CmdOk;

    }
    return CmdError;
}

void YModeCompletion::completeFromBuffer( YBuffer *buffer, QStringList &proposed, bool elimDups /*=true*/, QList<YCursor> *cursors /*=NULL*/ )
{
    // Guardian for empty buffers
    if ( buffer->isEmpty() ) {
        return ;
    }

    int matchedLength;
    bool found;
    YCursor matchCursor;
    YCursor nextCursor;
    YCursor endCursor;
    YZAction *action = buffer->action();
    const QString pattern = "\\b" + mPrefix + "\\w*";
    const YCursor bufbegin(0, 0);
    const YCursor bufend(0, buffer->lineCount() + 1);

    dbg() << "COMPLETION: pattern: " << pattern << endl;

    // set the initial search start
    nextCursor = bufbegin;
    endCursor = bufend;

    // loop through the buffer looking for matches
    // at each match we set nextCursor to the place we would have scanned next if
    // we didn't find the match.  This lets us continue through the entire buffer
    do {
        // search from the end of the prefix to the end of the buffer
        matchCursor = action->search(buffer, pattern, nextCursor, endCursor, &matchedLength, &found);
        nextCursor = YCursor( matchCursor.x() + matchedLength, matchCursor.y() );

        if ( found ) {
            QString possible = buffer->getWordAt( matchCursor );
            // add to the proposed
            // the contains check is to ensure we don't add duplicates
            // This is O(n^2), but hopefully it won't kill us, since the
            // sets are likely small
            // but only do the contains check if we're to eliminate duplicates
            if ( !elimDups || (elimDups && proposed.contains( possible ) == 0 )) {
                proposed.push_back( possible );
                if ( cursors ) {
                    cursors->push_back( matchCursor );
                }
            }
        }
    } while ( found );

    dbg() << "COMPLETION: Found " << proposed.size() << " matches" << endl;
}

void YModeCompletion::completeFromOtherBuffers( YBuffer *skip, QStringList &proposed )
{
    // for each buffer, call completeFromBuffer
    YBufferList buffers = YSession::self()->buffers();
    for ( YBufferList::iterator itr = buffers.begin(); itr != buffers.end(); ++itr ) {
        YBuffer *cur = *itr;

        dbg() << "COMPLETION: Inspecting another buffer" << endl;

        // don't descend into the buffer we're told to skip
        // this is done so that one buffer (probably the current)
        // has priority in the list
        if ( cur != skip ) {
            completeFromBuffer( cur, proposed );
        }
    }
}

void YModeCompletion::completeFromIncludes( QStringList & /*proposed*/ )
{}

void YModeCompletion::completeFromTags( QStringList &proposed )
{
    QStringList tags;
    tagStartsWith( mPrefix, tags );

    for ( int i = 0; i < tags.size(); ++i ) {
        if ( proposed.contains( tags[i] ) == 0 ) {
            proposed.push_back( tags[i] );
        }
    }
}

void YModeCompletion::completeFromDictionary( QStringList & /*proposed*/ )
{}

void YModeCompletion::completeFromFileNames( QStringList & /*proposed*/ )
{}

void YModeCompletion::completeFromCurrentBuffer( const YCursor cursor, bool forward, QStringList &proposed )
{
    YBuffer *buffer = YSession::self()->currentView()->myBuffer();

    QStringList matches;
    QList<YCursor> cursorlist;

    completeFromBuffer( buffer, matches, false, &cursorlist );

    // ASSERTION: when scanning the current buffer, we must
    //   find the search pattern at least once, since
    //   we'll find it where we typed
    //   Therefore: cursor must appear exactly once in
    //   the list cursors!
    YASSERT_MSG( cursorlist.contains( cursor ) == 1, "Current cursor not found in list of matched cursors" );

    // use the above fact to locate where in the cursors list
    // we should start scanning from
    int startidx = cursorlist.indexOf( cursor );

    int delta = forward ? 1 : -1; // direction to search through the list

    // now add all elements in matches to proposed, starting with the element
    // following startidx and wrapping all the way around
    // we loop until we've added matches.size() - 1 elements, because
    // we don't want to add the element at the cursor to the list
    for ( int count = 0, i = ( startidx + delta ) % matches.size();
            count < matches.size() - 1 && i >= 0;
            ++count, i = (i + delta) % matches.size() ) {
        if ( !proposed.contains( matches[ i ] ) ) {
            proposed.push_back( matches[ i ] );
        }
    }
}
