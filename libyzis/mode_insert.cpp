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
#include "mode_pool.h"
#include "portability.h"

#include "buffer.h"
#include "session.h"
#include "action.h"
#include "viewcursor.h"
#include "mode_complete.h"
#include "debug.h"
#include "view.h"

#define dbg()    yzDebug("YModeInsert")
#define err()    yzError("YModeInsert")

using namespace yzis;

YModeInsert::YModeInsert() : YModeCommand()
{
    mType = YMode::ModeInsert;
    mString = _( "[ Insert ]" );
    mIM = true;
    mMapMode = MapInsert;
    mIsEditMode = true;
    mIsCmdLineMode = false;
    mIsSelMode = false;
}

void YModeInsert::enter( YView* mView )
{
    mView->displayInfo("");
}

void YModeInsert::leave( YView* mView )
{
    if ( mView->getBufferCursor().x() > 0 )
		mView->gotoViewCursor(mView->moveHorizontal(-1));
}

void YModeInsert::initMotionPool()
{
    // No insert mode specific motions
}

void YModeInsert::initCommandPool()
{
    commands.append( new YCommand(YKeySequence("<ESC>"), &YModeCommand::gotoCommandMode) );
    commands.append( new YCommand(YKeySequence("<C-c>"), &YModeCommand::gotoCommandMode) );
    commands.append( new YCommand(YKeySequence("<C-[>"), &YModeCommand::gotoCommandMode) );
    commands.append( new YCommand(YKeySequence("<INSERT>"), (PoolMethod) &YModeInsert::commandInsert) );
    commands.append( new YCommand(YKeySequence("<ALT>"), &YModeCommand::gotoExMode) );
    commands.append( new YCommand(YKeySequence("<A-v>"), &YModeCommand::gotoVisualMode) );
    commands.append( new YCommand(YKeySequence("<C-e>"), (PoolMethod)&YModeInsert::insertFromBelow) );
    commands.append( new YCommand(YKeySequence("<C-y>"), (PoolMethod)&YModeInsert::insertFromAbove) );
    commands.append( new YCommand(YKeySequence("<C-x>"), (PoolMethod)&YModeInsert::completion) );
    commands.append( new YCommand(YKeySequence("<C-n>"), (PoolMethod)&YModeInsert::completionNext) );
    commands.append( new YCommand(YKeySequence("<C-p>"), (PoolMethod)&YModeInsert::completionPrevious) );
    commands.append( new YCommand(YKeySequence("<C-w>"), (PoolMethod)&YModeInsert::deleteWordBefore) );
    commands.append( new YCommand(YKeySequence("<C-u>"), (PoolMethod)&YModeInsert::deleteLineBefore) );
    commands.append( new YCommand(YKeySequence("<BS>"), (PoolMethod)&YModeInsert::backspace) );
    commands.append( new YCommand(YKeySequence("<C-h>"), (PoolMethod)&YModeInsert::backspace) );
    commands.append( new YCommand(YKeySequence("<ENTER>"), (PoolMethod)&YModeInsert::commandEnter) );
    commands.append( new YCommand(YKeySequence("<RETURN>"), (PoolMethod)&YModeInsert::commandEnter) );
    commands.append( new YCommand(YKeySequence("<DELETE>"), (PoolMethod)&YModeInsert::deleteChar) );
    
    // Motions don't need special processing in insert mode, so shove them
    // on the back of the command list
    for(QList<YMotion *>::const_iterator mot = motions.begin(); mot != motions.end(); ++mot)
	commands.append(*mot);
    
    // Get rid of unnecessary motion list now
    motions.clear();
}

void YModeInsert::initModifierKeys()
{
    mModifierKeys << "<CTRL>c" << "<CTRL>e" << "<CTRL>n" << "<CTRL>p"
    << "<CTRL>x" << "<CTRL>y" << "<ALT>:" << "<ALT>v"
    << "<CTRL>[" << "<CTRL>h" << "<CTRL>w" << "<CTRL>u" << "<CTRL><HOME>"
    << "<CTRL><END>";
}
/*
 * if you add a command which use modifiers keys, add it in initModifierKeys too
 */
CmdState YModeInsert::execCommand( YView* mView, const YKeySequence& inputs, 
                                   YKeySequence::const_iterator & parsePos )
{
    CmdState ret;
    YCommand *c = parseCommand(inputs, parsePos);

    dbg() << HERE() << endl;

    if ( c != NULL ) { // We have a special command
        QList<QChar> regs;
        regs << Qt::Key_QuoteDbl;
        // 1 is for count, false for hadCount
//        dbg() << ((void * )(&YModeCommand::deleteChar));
//        dbg() << HERE() << " got a special command : "<< c->describe() << " with address " << (long int)(c->poolMethod())  << ", executing it" << endl;
#ifdef DEBUG
        dbg() << HERE() << " got a special command : "<< c->describe() << ", executing it" << endl;
#endif

        return (this->*(c->poolMethod()))  (YCommandArgs(c, mView, regs, 1, false, &inputs, &parsePos) );
    }


    dbg() << HERE() << " NO command found, inserting text" << endl;
    /* if ( key.startsWith("<CTRL>") ) // XXX no sense
       ret = YSession::self()->getCommandPool()->execCommand(mView, key);
       else*/

    // As all insert mode commands are one key, we know parsePos hasn't changed if we're here
    QString text;
    if ( *parsePos == Qt::Key_Tab ) {
        // expand a tab to [tabstop] spaces if 'expandtab' is set
        if (mView->getLocalBooleanOption("expandtab"))
            text.fill(' ', mView->getLocalIntegerOption("tabstop"));
        else
            text = "\t";
    }
    else
        text = parsePos->toString();
    ret = addText( mView, text );
    QStringList ikeys = mView->myBuffer()->getLocalListOption("indentkeys");
    if ( ikeys.contains(text) )
        YSession::self()->eventCall("INDENT_ON_KEY", mView);

    return ret;
}


CmdState YModeInsert::insertFromAbove( const YCommandArgs &args )
{
    QString c = args.view->getCharBelow( -1 );
    if ( ! c.isNull() )
        return addText( args.view, c );
    return CmdStopped;
}

CmdState YModeInsert::insertFromBelow( const YCommandArgs &args )
{
    QString c = args.view->getCharBelow( 1 );
    if ( ! c.isNull() )
        return addText( args.view, c );
    return CmdStopped;
}
CmdState YModeInsert::completion( const YCommandArgs &args )
{
    args.view->modePool()->push( ModeCompletion );
    return CmdOk;
}

CmdState YModeInsert::completionNext( const YCommandArgs &args )
{
    args.view->modePool()->push( ModeCompletion );
    YModeCompletion* c = static_cast<YModeCompletion*>( args.view->modePool()->current() );
    YKeySequence inputs("<C-n>");
    YKeySequence::const_iterator parsePos = inputs.begin();
    return c->execCommand(args.view, inputs, parsePos);
}
CmdState YModeInsert::completionPrevious( const YCommandArgs &args )
{
    args.view->modePool()->push( ModeCompletion );
    YModeCompletion* c = static_cast<YModeCompletion*>( args.view->modePool()->current() );
    YKeySequence inputs("<C-p>");
    YKeySequence::const_iterator parsePos = inputs.begin();
    return c->execCommand(args.view, inputs, parsePos);
}

CmdState YModeInsert::deleteWordBefore( const YCommandArgs &args )
{
    YCursor cur = args.view->getBufferCursor();
    YBuffer* mBuffer = args.view->myBuffer();
    if ( cur.x() == 0 && cur.y() > 0 && args.view->getLocalStringOption( "backspace" ).contains( "eol" ) ) {
        mBuffer->action()->mergeNextLine( args.view, cur.y() - 1 );
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
        mBuffer->action()->deleteChar( args.view, x, cur.y(), cur.x() - x );
    }
    return CmdOk;
}

CmdState YModeInsert::deleteLineBefore( const YCommandArgs &args )
{
    YCursor cur = args.view->getBufferCursor();
    YBuffer* mBuffer = args.view->myBuffer();
    if ( cur.x() == 0 && cur.y() > 0 && args.view->getLocalStringOption( "backspace" ).contains( "eol" ) ) {
        mBuffer->action()->mergeNextLine( args.view, cur.y() - 1 );
    } else {
        mBuffer->action()->deleteChar( args.view, 0, cur.y(), cur.x() );
    }
    return CmdOk;
}

CmdState YModeInsert::deleteChar( const YCommandArgs &args )
{
    dbg() << HERE() << endl ; 

    YCursor cur = args.view->getBufferCursor();
    YBuffer* mBuffer = args.view->myBuffer();
    if ( cur.x() == mBuffer->textline( cur.y() ).length()
            && args.view->getLocalStringOption( "backspace" ).contains( "eol" ) ) {
        mBuffer->action()->mergeNextLine( args.view, cur.y(), false );
    } else {
        mBuffer->action()->deleteChar( args.view, cur, 1 );
    }
    return CmdOk;
}

CmdState YModeInsert::backspace( const YCommandArgs &args )
{
    YCursor cur = args.view->getBufferCursor();
    YBuffer* mBuffer = args.view->myBuffer();
    if ( cur.x() == 0 && cur.y() > 0 && args.view->getLocalStringOption( "backspace" ).contains( "eol" ) ) {
        mBuffer->action()->mergeNextLine( args.view, cur.y() - 1 );
        //mBuffer->action()->deleteChar( mView, *mView->getBufferCursor(), 1 ); see bug #158
    } else if ( cur.x() > 0 ) {
        mBuffer->action()->deleteChar( args.view, cur.x() - 1, cur.y(), 1 );
    }
    return CmdOk;
}

CmdState YModeInsert::commandEnter( const YCommandArgs &args )
{
    YCursor cur = args.view->getBufferCursor();
    YBuffer* mBuffer = args.view->myBuffer();
    if ( args.view->getLocalBooleanOption("cindent") ) {
        args.view->indent();
    } else {
        mBuffer->action()->insertNewLine( args.view, cur );
        QStringList results = YSession::self()->eventCall("INDENT_ON_ENTER", args.view);
        if (results.count() > 0 ) {
            if (results[0].length() != 0) {
                mBuffer->action()->replaceLine( args.view, cur.y() + 1, results[0] + mBuffer->textline( cur.y() + 1 ).trimmed() );
                args.view->gotoxy(results[0].length(), cur.y() + 1);
            }
        }
    }
    args.view->stickToColumn();
    return CmdOk;
}
CmdState YModeInsert::addText( YView* mView, const QString& key )
{
    dbg() << HERE() << endl;
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
        YKey input;
        input.fromString(entry);
        YSession::self()->sendKey( mView, input );
    }
    m_imPreedit = entry;
}

void YModeInsert::imEnd( YView* mView, const QString& entry )
{
    imCompose( mView, entry );
    m_imPreedit = "";
}

CmdState YModeInsert::commandInsert( const YCommandArgs &args)
{
    args.view->modePool()->change( ModeReplace, false );
    return CmdOk;
}


/**
 * YModeReplace
 */

YModeReplace::YModeReplace() : YModeInsert()
{
    mType = ModeReplace;
    mString = _("[ Replace ]");
}

CmdState YModeReplace::commandInsert( const YCommandArgs &args )
{
    args.view->modePool()->change( ModeInsert, false );
    return CmdOk;
}

CmdState YModeReplace::backspace( const YCommandArgs &args )
{
	args.view->gotoViewCursor(args.view->moveHorizontal(-1));
    return CmdOk;
}

CmdState YModeReplace::addText( YView* mView, const QString &text )
{
    mView->myBuffer()->action()->replaceChar( mView, mView->getBufferCursor(), text );
    return CmdOk;
}

