/*  This file is part of the Yzis libraries
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
*  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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

#include "mode_search.h"
#include "mode_pool.h"
#include "debug.h"
#include "portability.h"
#include "action.h"
#include "view.h"
#include "buffer.h"
#include "history.h"
#include "search.h"
#include "selection.h"
#include "session.h"

#define dbg()    yzDebug("YModeSearch")
#define err()    yzError("YModeSearch")

using namespace yzis;

YModeSearch::YModeSearch() : YMode()
{
    mType = YMode::ModeSearch;
    mString = _( "[ Search ]" );
    mMapMode = MapCmdline;
    mHistory = new YZHistory;
    incSearchFound = false;
    mIsEditMode = false;
    mIsCmdLineMode = true;
    mIsSelMode = false;
}
YModeSearch::~YModeSearch()
{
    delete mHistory;
}
void YModeSearch::enter( YView* view )
{
    view->guiSetFocusCommandLine();
    view->guiSetCommandLineText( "" );
    mSearchBegin = view->getBufferCursor();
}
void YModeSearch::leave( YView* view )
{
    view->guiSetCommandLineText( "" ); 
    view->guiSetFocusMainWindow();
}

YCursor YModeSearch::replaySearch( YView* view, bool* found )
{
    return YSession::self()->search()->replayForward( view->buffer(), found, view->getBufferCursor() );
}
YCursor YModeSearch::search( YView* view, const QString& s, bool* found )
{
    return YSession::self()->search()->forward( view->buffer(), s, found, view->getBufferCursor() );
}
YCursor YModeSearch::search( YView* view, const QString& s, const YCursor begin, int* matchlength, bool* found )
{
    YCursor end( 0, view->buffer()->lineCount() - 1 );
    end.setX( view->buffer()->textline( end.y() ).length() );
    return view->buffer()->action()->search( view->buffer(), s, begin, end, matchlength, found );
}

void YModeSearch::initModifierKeys()
{
    mModifierKeys << "<ALT>:";
}
CmdState YModeSearch::execCommand( YView* view, const YKeySequence& keys, YKeySequence::const_iterator &parsePos )
{
    //XXX YSelection* searchSelection = view->getSelectionPool()->search();

    if ( *parsePos == Qt::Key_Enter || *parsePos == Qt::Key_Return) {
        QString what = view->guiGetCommandLineText();
        dbg() << "Current search: " << what;

        bool found = false;
        YCursor pos;
        if ( what.isEmpty() ) {
            pos = replaySearch( view, &found );
        } else {
            mHistory->addEntry( what );
            pos = search( view, what, &found );
            if ( view->getLocalBooleanOption( "incsearch" ) && incSearchFound ) {
                pos = incSearchResult;
                incSearchFound = false;
            }
        }
        if ( found ) {
            view->gotoLinePosition(pos.y() , pos.x());
        } else {
            view->displayInfo(_("Pattern not found: ") + what);
        }
        view->modePool()->pop();
        ++parsePos;
        return CmdOk;
    } else if ( *parsePos == Qt::Key_Down ) {
        mHistory->goForwardInTime();
        view->guiSetCommandLineText( mHistory->getEntry() );
        ++parsePos;
        return CmdOk;
    } else if ( *parsePos == Qt::Key_Left || *parsePos == Qt::Key_Right ) {
        ++parsePos;
        return CmdOk;
    } else if ( *parsePos == Qt::Key_Up ) {
        mHistory->goBackInTime();
        view->guiSetCommandLineText( mHistory->getEntry() );
        ++parsePos;
        return CmdOk;
    } else if ( *parsePos == YKey(Qt::Key_Colon, Qt::AltModifier) ) {
        view->modePool()->change( ModeEx );
        ++parsePos;
        return CmdOk;
    } else if ( *parsePos == Qt::Key_Escape
                || *parsePos == YKey(Qt::Key_C, Qt::ControlModifier) ) {
        if ( view->getLocalBooleanOption( "incsearch" ) ) {
            view->gotoLinePosition(mSearchBegin.y(), mSearchBegin.x());
            view->setPaintAutoCommit( false );
            incSearchFound = false;
            //view->sendXXXPaintEvent( searchSelection->map() );
            //XXX searchSelection->clear();
            view->commitPaintEvent();
        }
        view->modePool()->pop();
        ++parsePos;
        return CmdOk;
    } else if ( *parsePos == Qt::Key_Backspace ) {
        QString back = view->guiGetCommandLineText();
        if ( back.isEmpty() ) {
            view->modePool()->pop();
            ++parsePos;
            return CmdOk;
        }
        view->guiSetCommandLineText(back.remove(back.length() - 1, 1));
    } else {
        view->guiSetCommandLineText( view->guiGetCommandLineText() + parsePos->toString() );
    }

    if ( view->getLocalBooleanOption("incsearch") ) {
        view->setPaintAutoCommit( false );
        int matchlength;
        incSearchResult = search(view, view->guiGetCommandLineText(), mSearchBegin, &matchlength, &(incSearchFound));
        if ( incSearchFound ) {
            if ( view->getLocalBooleanOption("hlsearch") ) {
                YCursor endResult(incSearchResult );
                endResult.setX( endResult.x() + matchlength - 1 );
                //XXX  searchSelection->addInterval( YInterval(incSearchResult, endResult) );
                //view->sendXXXPaintEvent( searchSelection->map() );
            }
            view->gotoLinePositionAndStick(incSearchResult );
        } else {
            view->gotoLinePosition(mSearchBegin.y() , mSearchBegin.x());
            //view->sendXXXPaintEvent( searchSelection->map() );
            //XXX searchSelection->clear();
        }
        view->commitPaintEvent();
    }

    ++parsePos;
    return CmdOk;
}


YModeSearchBackward::YModeSearchBackward() : YModeSearch()
{
    mType = YMode::ModeSearchBackward;
    mString = _( "[ Search backward ]" );
}
YModeSearchBackward::~YModeSearchBackward()
{}

YCursor YModeSearchBackward::replaySearch( YView* view, bool * found )
{
    return YSession::self()->search()->replayBackward( view->buffer(), found, view->getBufferCursor() );
}
YCursor YModeSearchBackward::search( YView* view, const QString& s, bool* found )
{
    //XXX YCursor buffer = view->getBufferCursor();
    //XXX view->gotoLinePosition(false , buffer.x() + 1, buffer.y());
    return YSession::self()->search()->backward( view->buffer(), s, found, view->getBufferCursor() );
}
YCursor YModeSearchBackward::search( YView* view, const QString& s, const YCursor begin, int* matchlength, bool* found )
{
    YCursor end( 0, 0 );
    return view->buffer()->action()->search( view->buffer(), s, begin, end, matchlength, found );
}



