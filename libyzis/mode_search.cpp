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
}
YModeSearch::~YModeSearch()
{
    delete mHistory;
}
void YModeSearch::enter( YView* view )
{
    YSession::self()->guiSetFocusCommandLine();
    view->guiSetCommandLineText( "" );
    mSearchBegin = view->getBufferCursor();
}
void YModeSearch::leave( YView* view )
{
    view->guiSetCommandLineText( "" );
    YSession::self()->guiSetFocusMainWindow();
}

YCursor YModeSearch::replaySearch( YView* view, bool* found )
{
    return YSession::self()->search()->replayForward( view->myBuffer(), found, view->getBufferCursor() );
}
YCursor YModeSearch::search( YView* view, const QString& s, bool* found )
{
    return YSession::self()->search()->forward( view->myBuffer(), s, found, view->getBufferCursor() );
}
YCursor YModeSearch::search( YView* view, const QString& s, const YCursor begin, int* matchlength, bool* found )
{
    YCursor end( 0, view->myBuffer()->lineCount() - 1 );
    end.setX( view->myBuffer()->textline( end.y() ).length() );
    return view->myBuffer()->action()->search( view->myBuffer(), s, begin, end, matchlength, found );
}

void YModeSearch::initModifierKeys()
{
    mModifierKeys << "<ALT>:";
}
CmdState YModeSearch::execCommand( YView* view, const QString& _key )
{
    QString key = _key;
    YSelection* searchSelection = view->getSelectionPool()->search();

    if ( key == "<ENTER>" ) {
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
            view->gotoxy( pos.x(), pos.y() );
        } else {
            view->guiDisplayInfo(_("No match"));
        }
        view->modePool()->pop();
        return CmdOk;
    } else if ( key == "<DOWN>" ) {
        mHistory->goForwardInTime();
        view->guiSetCommandLineText( mHistory->getEntry() );
        return CmdOk;
    } else if ( key == "<LEFT>" || key == "<RIGHT>" ) {
        return CmdOk;
    } else if ( key == "<UP>" ) {
        mHistory->goBackInTime();
        view->guiSetCommandLineText( mHistory->getEntry() );
        return CmdOk;
    } else if ( key == "<ALT>:" ) {
        view->modePool()->change( ModeEx );
        return CmdOk;
    } else if ( key == "<ESC>" || key == "<CTRL>c" ) {
        if ( view->getLocalBooleanOption( "incsearch" ) ) {
            view->gotoxy(mSearchBegin.x(), mSearchBegin.y());
            view->setPaintAutoCommit( false );
            incSearchFound = false;
            view->sendPaintEvent( searchSelection->map() );
            searchSelection->clear();
            view->commitPaintEvent();
        }
        view->modePool()->pop();
        return CmdOk;
    } else if ( key == "<BS>" ) {
        QString back = view->guiGetCommandLineText();
        if ( back.isEmpty() ) {
            view->modePool()->pop();
            return CmdOk;
        }
        view->guiSetCommandLineText(back.remove(back.length() - 1, 1));
    } else {
        view->guiSetCommandLineText( view->guiGetCommandLineText() + key );
    }

    if ( view->getLocalBooleanOption("incsearch") ) {
        view->setPaintAutoCommit( false );
        int matchlength;
        incSearchResult = search(view, view->guiGetCommandLineText(), mSearchBegin, &matchlength, &(incSearchFound));
        if ( incSearchFound ) {
            if ( view->getLocalBooleanOption("hlsearch") ) {
                YCursor endResult(incSearchResult );
                endResult.setX( endResult.x() + matchlength - 1 );
                searchSelection->addInterval( YInterval(incSearchResult, endResult) );
                view->sendPaintEvent( searchSelection->map() );
            }
            view->gotoxyAndStick(incSearchResult );
        } else {
            view->gotoxy( mSearchBegin.x(), mSearchBegin.y() );
            view->sendPaintEvent( searchSelection->map() );
            searchSelection->clear();
        }
        view->commitPaintEvent();
    }
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
    return YSession::self()->search()->replayBackward( view->myBuffer(), found, view->getBufferCursor() );
}
YCursor YModeSearchBackward::search( YView* view, const QString& s, bool* found )
{
    YCursor buffer = view->getBufferCursor();
    view->gotoxy( buffer.x() + 1, buffer.y(), false );
    return YSession::self()->search()->backward( view->myBuffer(), s, found, view->getBufferCursor() );
}
YCursor YModeSearchBackward::search( YView* view, const QString& s, const YCursor begin, int* matchlength, bool* found )
{
    YCursor end( 0, 0 );
    return view->myBuffer()->action()->search( view->myBuffer(), s, begin, end, matchlength, found );
}



