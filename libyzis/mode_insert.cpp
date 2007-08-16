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

#define dbg()    yzDebug("YModeInsert")
#define err()    yzError("YModeInsert")

using namespace yzis;

YModeInsert::YModeInsert() : YMode()
{
    mType = YMode::ModeInsert;
    mString = _( "[ Insert ]" );
    mEditMode = true;
    mIM = true;
    mMapMode = MapInsert;
}
void YModeInsert::leave( YView* mView )
{
    if ( mView->getBufferCursor().x() > 0 )
        mView->moveLeft();
}

void YModeInsert::initModifierKeys()
{
    mModifierKeys << "<CTRL>c" << "<CTRL>e" << "<CTRL>n" << "<CTRL>p"
    << "<CTRL>x" << "<CTRL>y" << "<ALT>:" << "<ALT>v"
    << "<CTRL>[" << "<CTRL>h" << "<CTRL>w" ;
}
/*
 * if you add a command which use modifiers keys, add it in initModifierKeys too
 */
CmdState YModeInsert::execCommand( YView* mView, const QString& _key )
{
    QString key = _key;
    CmdState ret = CmdOk;
    if ( key == "<HOME>" ) commandHome( mView, key );
    else if ( key == "<END>" ) commandEnd( mView, key );
    else if ( key == "<ESC>"
              || key == "<CTRL>c"
              || key == "<CTRL>[" ) commandEscape( mView, key );
    else if ( key == "<INS>" ) commandInsert( mView, key );
    else if ( key == "<ALT>:" ) commandEx( mView, key );
    else if ( key == "<ALT>v" ) commandVisual( mView, key );
    else if ( key == "<DOWN>" ) commandDown( mView, key );
    else if ( key == "<LEFT>" ) commandLeft( mView, key );
    else if ( key == "<RIGHT>" ) commandRight( mView, key );
    else if ( key == "<UP>" ) commandUp( mView, key );
    else if ( key == "<PDOWN>" ) commandPageDown( mView, key );
    else if ( key == "<PUP>" ) commandPageUp( mView, key );
    else if ( key == "<CTRL>e" ) commandInsertFromBelow( mView, key );
    else if ( key == "<CTRL>y" ) commandInsertFromAbove( mView, key );
    // completion
    else if ( key == "<CTRL>x" ) commandCompletion( mView, key );
    else if ( key == "<CTRL>n" ) commandCompletionNext( mView, key );
    else if ( key == "<CTRL>p" ) commandCompletionPrevious( mView, key );
    else if ( key == "<CTRL>w" ) commandDeleteWordBefore( mView, key );
    else if ( key == "<BS>"
              || key == "<CTRL>h") commandBackspace( mView, key );
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
         ret = YSession::self()->getCommandPool()->execCommand(mView, key);
        else*/
        ret = commandDefault( mView, key );
        QStringList ikeys = mView->myBuffer()->getLocalListOption("indentkeys");
        if ( ikeys.contains(key) )
            YSession::self()->eventCall("INDENT_ON_KEY", mView);
    }
    return ret;
}

void YModeInsert::commandHome( YView* mView, const QString& )
{
    mView->moveToStartOfLine();
}
void YModeInsert::commandEnd( YView* mView, const QString& )
{
    mView->moveToEndOfLine();
}
void YModeInsert::commandEscape( YView* mView, const QString& )
{
    mView->modePool()->pop( ModeCommand );
}
void YModeInsert::commandInsert( YView* mView, const QString& )
{
    mView->modePool()->change( ModeReplace, false );
}
void YModeInsert::commandEx( YView* mView, const QString& )
{
    mView->modePool()->push( ModeEx );
}
void YModeInsert::commandVisual( YView* mView, const QString& )
{
    mView->modePool()->push( ModeVisual );
}
void YModeInsert::commandInsertFromAbove( YView* mView, const QString& )
{
    QString c = mView->getCharBelow( -1 );
    if ( ! c.isNull() )
        commandDefault( mView, c );
}
void YModeInsert::commandInsertFromBelow( YView* mView, const QString& )
{
    QString c = mView->getCharBelow( 1 );
    if ( ! c.isNull() )
        commandDefault( mView, c );
}
void YModeInsert::commandCompletion( YView* mView, const QString& )
{
    mView->modePool()->push( ModeCompletion );
}
void YModeInsert::commandCompletionNext( YView* mView, const QString& )
{
    mView->modePool()->push( ModeCompletion );
    YModeCompletion* c = static_cast<YModeCompletion*>( mView->modePool()->current() );
    c->execCommand(mView, "<CTRL>n");
}
void YModeInsert::commandCompletionPrevious( YView* mView, const QString& )
{
    mView->modePool()->push( ModeCompletion );
    YModeCompletion* c = static_cast<YModeCompletion*>( mView->modePool()->current() );
    c->execCommand(mView, "<CTRL>p");
}
void YModeInsert::commandDown( YView* mView, const QString& )
{
    mView->moveDown();
}
void YModeInsert::commandUp( YView* mView, const QString& )
{
    mView->moveUp();
}
void YModeInsert::commandLeft( YView* mView, const QString& )
{
    mView->moveLeft();
}
void YModeInsert::commandRight( YView* mView, const QString& )
{
    mView->moveRight();
}
void YModeInsert::commandPageDown( YView* mView, const QString& )
{
    int line = mView->getCurrentTop() + mView->getLinesVisible();

    if (mView->getLocalBooleanOption("wrap")) {
        YViewCursor temp = mView->viewCursor();
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
void YModeInsert::commandPageUp( YView* mView, const QString& )
{
    int line = mView->getCurrentTop() - mView->getLinesVisible();

    if (line < 0)
        line = 0;

    if (line != (int)mView->getCurrentTop()) {
        mView->alignViewBufferVertically( line );
        mView->moveToFirstNonBlankOfLine();
    }
}
void YModeInsert::commandBackspace( YView* mView, const QString& )
{
    YCursor cur = mView->getBufferCursor();
    YBuffer* mBuffer = mView->myBuffer();
    if ( cur.x() == 0 && cur.y() > 0 && mView->getLocalStringOption( "backspace" ).contains( "eol" ) ) {
        mBuffer->action()->mergeNextLine( mView, cur.y() - 1 );
        //mBuffer->action()->deleteChar( mView, *mView->getBufferCursor(), 1 ); see bug #158
    } else if ( cur.x() > 0 ) {
        mBuffer->action()->deleteChar( mView, cur.x() - 1, cur.y(), 1 );
    }
}
void YModeInsert::commandDeleteWordBefore( YView* mView, const QString& )
{
    YCursor cur = mView->getBufferCursor();
    YBuffer* mBuffer = mView->myBuffer();
    if ( cur.x() == 0 && cur.y() > 0 && mView->getLocalStringOption( "backspace" ).contains( "eol" ) ) {
        mBuffer->action()->mergeNextLine( mView, cur.y() - 1 );
        //mBuffer->action()->deleteChar( mView, *mView->getBufferCursor(), 1 ); see bug #158
    } else {
        QString line = mBuffer->textline( cur.y() );
        QChar tmp;
        bool isWord;

        int x = cur.x();
        // delete whitespace characters preceding current word
        while ( x > 0 && line[x - 1].isSpace() )
            --x;

        // delete a word or set of not-word not-whitespace characters
        if ( x > 0 ) {
            tmp = line[x - 1];
            isWord = tmp.isLetterOrNumber() || tmp == '_' || tmp.isMark();

            // delete word behind the cursor (if there is one)
            if ( isWord )
                while ( isWord && --x > 0 ) {
                    tmp = line[x - 1];
                    isWord = ( tmp.isLetterOrNumber() || tmp == '_' || tmp.isMark() );
                }

            // otherwise, delete all not-word and not-whitespace
            // characters behind the cursor
            else
                while ( !isWord && !tmp.isSpace() && --x > 0 ) {
                    tmp = line[x - 1];
                    isWord = ( tmp.isLetterOrNumber() || tmp == '_' || tmp.isMark() );
                }
        }

        //do it
        mBuffer->action()->deleteChar( mView, x, cur.y(), cur.x() - x );
    }
}
void YModeInsert::commandDel( YView* mView, const QString& )
{
    YCursor cur = mView->getBufferCursor();
    YBuffer* mBuffer = mView->myBuffer();
    if ( cur.x() == mBuffer->textline( cur.y() ).length()
            && mView->getLocalStringOption( "backspace" ).contains( "eol" ) ) {
        mBuffer->action()->mergeNextLine( mView, cur.y(), false );
    } else {
        mBuffer->action()->deleteChar( mView, cur, 1 );
    }
}
void YModeInsert::commandEnter( YView* mView, const QString& )
{
    YCursor cur = mView->getBufferCursor();
    YBuffer* mBuffer = mView->myBuffer();
    if ( mView->getLocalBooleanOption("cindent") ) {
        mView->indent();
    } else {
        mBuffer->action()->insertNewLine( mView, cur );
        QStringList results = YSession::self()->eventCall("INDENT_ON_ENTER", mView);
        if (results.count() > 0 ) {
            if (results[0].length() != 0) {
                mBuffer->action()->replaceLine( mView, cur.y() + 1, results[0] + mBuffer->textline( cur.y() + 1 ).trimmed() );
                mView->gotoxy(results[0].length(), cur.y() + 1);
            }
        }
    }
    mView->updateStickyCol();
}
CmdState YModeInsert::commandDefault( YView* mView, const QString& key )
{
    mView->myBuffer()->action()->insertChar( mView, mView->getBufferCursor(), key );
    if ( mView->getLocalBooleanOption( "cindent" ) && key == "}" )
        mView->reindent( QPoint(mView->getBufferCursor().x() - 1, mView->getBufferCursor().y()));
    return CmdOk;
}

void YModeInsert::imBegin( YView* )
{
    m_imPreedit = "";
}
void YModeInsert::imCompose( YView* mView, const QString& entry )
{
    if ( !m_imPreedit.isEmpty() ) { // replace current one
        YCursor pos = mView->getBufferCursor();
        int len = m_imPreedit.length();
        if ( pos.x() >= len )
            pos.setX( pos.x() - len );
        else
            pos.setX( 0 );
        mView->myBuffer()->action()->replaceText( mView, pos, len, entry );
    } else {
        YSession::self()->sendKey( mView, entry );
    }
    m_imPreedit = entry;
}
void YModeInsert::imEnd( YView* mView, const QString& entry )
{
    imCompose( mView, entry );
    m_imPreedit = "";
}


/**
 * YModeReplace
 */

YModeReplace::YModeReplace() : YModeInsert()
{
    mType = ModeReplace;
    mString = _("[ Replace ]");
}

void YModeReplace::commandInsert( YView* mView, const QString& )
{
    mView->modePool()->change( ModeInsert, false );
}
void YModeReplace::commandBackspace( YView* mView, const QString& key )
{
    commandLeft( mView, key );
}

CmdState YModeReplace::commandDefault( YView* mView, const QString& key )
{
    mView->myBuffer()->action()->replaceChar( mView, mView->getBufferCursor(), key );
    return CmdOk;
}

