/*  This file is part of the Yzis libraries
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
*  Copyright (C) 2003-2004 Philippe Fremy <phil@freehackers.org>
*  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
*  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
*  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
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

#include "mode_command.h"
#include "portability.h"

#include <QRegExp>

#include "debug.h"

#include "action.h"
#include "buffer.h"
#include "cursor.h"
#include "linesearch.h"
#include "mark.h"
#include "search.h"
#include "session.h"
#include "tags_interface.h"
#include "view.h"
#include "viewcursor.h"

#define dbg()    yzDebug("YModeCommand")
#define err()    yzError("YModeCommand")

using namespace yzis;

YModeCommand::YModeCommand() : YMode()
{
    mType = ModeCommand;
    mString = _( "[ Awaiting Command ]" );
    commands.clear();
}

YModeCommand::~YModeCommand()
{
    for ( int ab = 0 ; ab < commands.size(); ++ab)
        delete commands.at(ab);
    commands.clear();
}
void YModeCommand::init()
{
    initPool();
    initModifierKeys();
}

void YModeCommand::initPool()
{
    initMotionPool();
    initCommandPool();
}
void YModeCommand::initMotionPool()
{
    commands.append( new YMotion("0", &YModeCommand::gotoSOL, ArgNone) );
    commands.append( new YMotion("$", &YModeCommand::gotoEOL, ArgNone) );
    commands.append( new YMotion("^", &YModeCommand::firstNonBlank, ArgNone) );
    commands.append( new YMotion("w", &YModeCommand::moveWordForward, ArgNone) );
    commands.append( new YMotion("W", &YModeCommand::moveSWordForward, ArgNone) );
    commands.append( new YMotion("b", &YModeCommand::moveWordBackward, ArgNone) );
    commands.append( new YMotion("B", &YModeCommand::moveSWordBackward, ArgNone) );
    commands.append( new YMotion("j", &YModeCommand::moveDown, ArgNone) );
    commands.append( new YMotion("k", &YModeCommand::moveUp, ArgNone) );
    commands.append( new YMotion("h", &YModeCommand::moveLeft, ArgNone) );
    commands.append( new YMotion("l", &YModeCommand::moveRight, ArgNone) );
    commands.append( new YMotion("<BS>", &YModeCommand::moveLeftWrap, ArgNone) );
    commands.append( new YMotion(" ", &YModeCommand::moveRightWrap, ArgNone) );
    commands.append( new YMotion("f", &YModeCommand::findNext, ArgChar) );
    commands.append( new YMotion("t", &YModeCommand::findBeforeNext, ArgChar) );
    commands.append( new YMotion("F", &YModeCommand::findPrevious, ArgChar) );
    commands.append( new YMotion("T", &YModeCommand::findAfterPrevious, ArgChar) );
    commands.append( new YMotion(";", &YModeCommand::repeatFind, ArgChar) );
    commands.append( new YMotion("*", &YModeCommand::searchWord, ArgNone) );
    commands.append( new YMotion("g*", &YModeCommand::searchWord, ArgNone) );
    commands.append( new YMotion("#", &YModeCommand::searchWord, ArgNone) );
    commands.append( new YMotion("g#", &YModeCommand::searchWord, ArgNone) );
    commands.append( new YMotion("n", &YModeCommand::searchNext, ArgNone) );
    commands.append( new YMotion("N", &YModeCommand::searchPrev, ArgNone) );
    commands.append( new YMotion("<HOME>", &YModeCommand::gotoSOL, ArgNone) );
    commands.append( new YMotion("<END>", &YModeCommand::gotoEOL, ArgNone) );
    commands.append( new YMotion("<CTRL><HOME>", &YModeCommand::gotoStartOfDocument, ArgNone) );
    commands.append( new YMotion("<CTRL><END>", &YModeCommand::gotoEndOfDocument, ArgNone) );
    commands.append( new YMotion("<LEFT>", &YModeCommand::moveLeft, ArgNone) );
    commands.append( new YMotion("<RIGHT>", &YModeCommand::moveRight, ArgNone) );
    commands.append( new YMotion("<UP>", &YModeCommand::moveUp, ArgNone) );
    commands.append( new YMotion("<DOWN>", &YModeCommand::moveDown, ArgNone) );
    commands.append( new YMotion("%", &YModeCommand::matchPair, ArgNone) );
    commands.append( new YMotion("`", &YModeCommand::gotoMark, ArgMark) );
    commands.append( new YMotion("'", &YModeCommand::gotoMark, ArgMark) );
    commands.append( new YMotion("<ENTER>", &YModeCommand::firstNonBlankNextLine, ArgNone) );
    commands.append( new YMotion("gg", &YModeCommand::gotoLine, ArgNone) );
    commands.append( new YMotion("G", &YModeCommand::gotoLine, ArgNone) );
    commands.append( new YMotion("}", &YModeCommand::nextEmptyLine, ArgNone) );
    commands.append( new YMotion("{", &YModeCommand::previousEmptyLine, ArgNone) );
}

void YModeCommand::initCommandPool()
{
    commands.append( new YCommand("I", &YModeCommand::insertAtSOL) );
    commands.append( new YCommand("gI", &YModeCommand::insertAtCol1) );
    commands.append( new YCommand("i", &YModeCommand::gotoInsertMode) );
    commands.append( new YCommand("<INS>", &YModeCommand::gotoInsertMode) );
    commands.append( new YCommand(":", &YModeCommand::gotoExMode) );
    commands.append( new YCommand("R", &YModeCommand::gotoReplaceMode) );
    commands.append( new YCommand("v", &YModeCommand::gotoVisualMode) );
    commands.append( new YCommand("V", &YModeCommand::gotoVisualLineMode) );
    commands.append( new YCommand("<CTRL>v", &YModeCommand::gotoVisualBlockMode) );
    commands.append( new YCommand("z<ENTER>", &YModeCommand::gotoLineAtTop) );
    commands.append( new YCommand("z+", &YModeCommand::gotoLineAtTop) );
    commands.append( new YCommand("z.", &YModeCommand::gotoLineAtCenter) );
    commands.append( new YCommand("z-", &YModeCommand::gotoLineAtBottom) );
    commands.append( new YCommand("dd", &YModeCommand::deleteLine) );
    commands.append( new YCommand("dG", &YModeCommand::deleteToEndOfLastLine) );
    commands.append( new YCommand("d", &YModeCommand::del, ArgMotion) );
    commands.append( new YCommand("D", &YModeCommand::deleteToEOL) );
    commands.append( new YCommand("s", &YModeCommand::substitute) );
    commands.append( new YCommand("x", &YModeCommand::deleteChar) );
    commands.append( new YCommand("X", &YModeCommand::deleteCharBackwards) );
    commands.append( new YCommand("yy", &YModeCommand::yankLine) );
    commands.append( new YCommand("y", &YModeCommand::yank, ArgMotion) );
    commands.append( new YCommand("Y", &YModeCommand::yankToEOL) );
    commands.append( new YCommand("cc", &YModeCommand::changeLine) );
    commands.append( new YCommand("S", &YModeCommand::changeLine) );
    commands.append( new YCommand("c", &YModeCommand::change, ArgMotion) );
    commands.append( new YCommand("C", &YModeCommand::changeToEOL) );
    commands.append( new YCommand("p", &YModeCommand::pasteAfter) );
    commands.append( new YCommand("P", &YModeCommand::pasteBefore) );
    commands.append( new YCommand("o", &YModeCommand::insertLineAfter) );
    commands.append( new YCommand("O", &YModeCommand::insertLineBefore) );
    commands.append( new YCommand("a", &YModeCommand::append) );
    commands.append( new YCommand("A", &YModeCommand::appendAtEOL) );
    commands.append( new YCommand("J", &YModeCommand::joinLine) );
    commands.append( new YCommand("gJ", &YModeCommand::joinLineWithoutSpace) );
    commands.append( new YCommand("<", &YModeCommand::indent, ArgMotion ) );
    commands.append( new YCommand("<<", &YModeCommand::indent ) );
    commands.append( new YCommand(">", &YModeCommand::indent, ArgMotion ) );
    commands.append( new YCommand(">>", &YModeCommand::indent ) );
    commands.append( new YCommand("ZZ", &YModeCommand::saveAndClose) );
    commands.append( new YCommand("ZQ", &YModeCommand::closeWithoutSaving) );
    commands.append( new YCommand("/", &YModeCommand::searchForwards) );
    commands.append( new YCommand("?", &YModeCommand::searchBackwards) );
    commands.append( new YCommand("~", &YModeCommand::changeCase) );
    commands.append( new YCommand("m", &YModeCommand::mark, ArgChar) );
    commands.append( new YCommand("r", &YModeCommand::replace, ArgChar) );
    commands.append( new YCommand("u", &YModeCommand::undo) );
    commands.append( new YCommand("U", &YModeCommand::redo) );
    commands.append( new YCommand("<CTRL>r", &YModeCommand::redo) );
    commands.append( new YCommand("q", &YModeCommand::macro) );
    commands.append( new YCommand("@", &YModeCommand::replayMacro) );
    commands.append( new YCommand("<CTRL>l", &YModeCommand::redisplay) );
    commands.append( new YCommand("<CTRL>[", &YModeCommand::gotoCommandMode) );
    commands.append( new YCommand("<ESC>", &YModeCommand::abort) );
    commands.append( new YCommand("<CTRL>c", &YModeCommand::abort) );
    commands.append( new YCommand("<DEL>", &YModeCommand::delkey) );
    commands.append( new YCommand("<ALT>:", &YModeCommand::gotoExMode) );
    commands.append( new YCommand("gUU", &YModeCommand::lineToUpperCase) );
    commands.append( new YCommand("gUgU", &YModeCommand::lineToUpperCase) );
    commands.append( new YCommand("guu", &YModeCommand::lineToLowerCase) );
    commands.append( new YCommand("gugu", &YModeCommand::lineToLowerCase) );
    commands.append( new YCommand("<PUP>", &YModeCommand::scrollPageUp) );
    commands.append( new YCommand("<CTRL>b", &YModeCommand::scrollPageUp) );
    commands.append( new YCommand("<CTRL>y", &YModeCommand::scrollLineUp) );
    commands.append( new YCommand("<PDOWN>", &YModeCommand::scrollPageDown) );
    commands.append( new YCommand("<CTRL>f", &YModeCommand::scrollPageDown) );
    commands.append( new YCommand("<CTRL>e", &YModeCommand::scrollLineDown) );
    commands.append( new YCommand(".", &YModeCommand::redoLastCommand) );
    commands.append( new YCommand("<CTRL>]", &YModeCommand::tagNext) );
    commands.append( new YCommand("<CTRL>t", &YModeCommand::tagPrev) );
    commands.append( new YCommand("<CTRL>o", &YModeCommand::undoJump) );
    commands.append( new YCommand("<CTRL>a", &YModeCommand::incrementNumber) );
    commands.append( new YCommand("<CTRL>x", &YModeCommand::decrementNumber) );
}

void YModeCommand::initModifierKeys()
{
    for ( int ab = 0 ; ab < commands.size(); ++ab) {
        const QString& keys = commands.at(ab)->keySeq();
        if ( keys.indexOf( "<SHIFT>" ) > -1 || keys.indexOf( "<CTRL>" ) > -1 || keys.indexOf( "<ALT>" ) > -1 ) {
            mModifierKeys << keys;
        }
    }
}

CmdState YModeCommand::execCommand(YView *view, const QString& inputs)
{
    // dbg() << "ExecCommand : " << inputs << endl;

    int count = 1;
    bool hadCount = false;
    int i = 0;
    QList<QChar> regs;

    // read in the register operations and the counts
    while (i < inputs.length()) {
        if (inputs.at( i ).digitValue() > 0) {
            int j = i + 1;
            while (j < inputs.length() && inputs.at(j).digitValue() >= 0)
                j++;
            count *= inputs.mid(i, j - i).toInt();
            i = j;
            dbg() << "Count " << count << endl;
            hadCount = true; //we found digits given by the user
        } else if (inputs.at( i ) == '\"') {
            if (++i >= inputs.length())
                break;
            regs << inputs.at(i++);
        } else
            break;
    }
    //if regs is empty add the default register
    if ( regs.count() == 0 )
        regs << '\"';

    if (i >= inputs.length())
        return CmdNotYetValid;

    // collect all the commands
    QList<YCommand*> cmds, prevcmds;

    int j = i;

    // retrieve all the matching commands
    // .. first the ones whose first key matches
    if (j < inputs.length()) {
        for (int ab = 0; ab < commands.size(); ++ab )
            if (commands.at(ab)->keySeq().startsWith(inputs.mid(j, 1)))
                cmds.append(commands.at(ab));
    }
    j++;
    // .. then the ones whose next keys match, too
    while (!cmds.isEmpty() && ++j <= inputs.length()) {
        prevcmds = cmds;
        // delete all the commands that don't match
        for ( int bc = 0 ; bc < cmds.size() ; )
            if (cmds.at(bc)->keySeq().startsWith(inputs.mid(i, j - i)))
                ++bc;
            else
                cmds.removeAt(bc);
    }
    if (cmds.isEmpty()) {
        // perhaps it is a command with an argument, isolate all those
        for ( int bc = 0 ; bc < prevcmds.size() ; )
            if ( prevcmds.at(bc)->arg() == ArgNone )
                prevcmds.removeAt(bc);
            else
                ++bc;
        if (prevcmds.isEmpty())
            return CmdError;
        // it really is a command with an argument, read it in
        const YCommand *c = prevcmds.first();
        i = j - 1;
        // read in a count that may follow
        if (c->arg() == ArgChar) { // don't try to read a motion!
            (this->*(c->poolMethod()))(YCommandArgs(c, view, regs, count, hadCount, inputs.mid(i)));
            return CmdOk;
        }
        if (inputs.at(i).digitValue() > 0) {
            while (j < inputs.length() && inputs.at(j).digitValue() > 0)
                j++;
            count *= inputs.mid(i, j - i).toInt();
            i = j;
            if (i >= inputs.length() )
                return CmdOperatorPending;
        }

        QString s = inputs.mid(i);
        switch (c->arg()) {
        case ArgMotion:
            if (s[0] == 'a' || s[0] == 'i') {
                // text object
                if (s.length() == 1)
                    return CmdOperatorPending;
                else if (!textObjects.contains(s))
                    return CmdError;
            } else {
                bool matched = false;
                for (int ab = 0; ab < commands.size(); ++ab) {
                    const YMotion *m = dynamic_cast<const YMotion*>(commands.at(ab));
                    if (m && m->matches(s)) {
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    for (int ab = 0; ab < commands.size(); ++ab ) {
                        const YMotion *m = dynamic_cast<const YMotion*>(commands.at(ab));
                        if (m && m->matches(s, false))
                            return CmdOperatorPending;

                    }
                }
            }
            break;
        case ArgChar:
        case ArgReg:
            if (s.length() != 1)
                return CmdError;
            break;
        case ArgMark:
            if (s.length() != 1 || !YCommand::isMark(s[0]))
                return CmdError;
            break;
        default:
            break;
        }

        // the argument is OK, go for it
        foreach( YView *v, view->myBuffer()->views() )
        v->setPaintAutoCommit( false );

        (this->*(c->poolMethod()))(YCommandArgs(c, view, regs, count, hadCount, s));

        foreach( YView *v, view->myBuffer()->views() )
        v->commitPaintEvent();

        if ( c->arg() == ArgMark ) {
            YSession::self()->saveJumpPosition();
        }

    } else {
        // keep the commands that match exactly
        QString s = inputs.mid(i);
        for ( int ab = 0 ; ab < cmds.size(); ) {
            if (cmds.at(ab)->keySeq() != s)
                cmds.removeAt(ab);
            else
                ++ab;
        }
        if (cmds.isEmpty())
            return CmdNotYetValid;
        const YCommand *c = 0;
        if (cmds.count() == 1) {
            c = cmds.first();
            if (c->arg() == ArgNone)
                (this->*(c->poolMethod()))(YCommandArgs(c, view, regs, count, hadCount));
            else
                return CmdOperatorPending;
        } else {
            /* two or more commands with the same name, we assert that these are exactly
            a cmd that needs a motion and one without an argument. In visual mode, we take
            the operator, in normal mode, we take the other. */ 
            //this is not sufficient, see the 'q' (record macro command), we need a q+ArgChar and a 'q' commands //mm //FIXME
            for ( int ab = 0 ; ab < cmds.size(); ++ab ) {
                if ( cmds.at(ab)->arg() == ArgMotion && MapVisual ||
                        cmds.at(ab)->arg() == ArgNone && !MapVisual )
                    c = cmds.at(ab);
            }
            if (!c)
                return CmdError;

            foreach( YView *v, view->myBuffer()->views() )
            v->setPaintAutoCommit( false );
            (this->*(c->poolMethod()))(YCommandArgs(c, view, regs, count, hadCount, QString()));
            foreach( YView *v, view->myBuffer()->views() )
            v->commitPaintEvent();
        }
    }

    return CmdOk;
}

bool YMotion::matches(const QString &s, bool fully) const
{
    QString ks = mKeySeq;
    if (s.startsWith(ks)) {
        switch (mArg) {
        case ArgNone:
            if (s.length() == ks.length())
                return true;
            break;
        case ArgChar:
            if (s.length() == ks.length() + 1 || !fully && s.length() == ks.length())
                return true;
            break;
        case ArgMark:
            if (s.length() == ks.length() + 1 && isMark(s.at(s.length() - 1)) || !fully && s.length() == ks.length())
                return true;
            break;
        default:
            break;
        }
    } else if (!fully && ks.startsWith(s))
        return true;

    return false;
}

YCursor YModeCommand::move(YView *view, const QString &inputs, int count, bool usercount)
{
    for (int ab = 0 ; ab < commands.size(); ++ab ) {
        const YMotion *m = dynamic_cast<const YMotion*>(commands.at(ab));
        if (m && m->matches(inputs)) {
            // execute the corresponding method
            YCursor to = (this->*(m->motionMethod()))(YMotionArgs(view, count, inputs.right( m->keySeq().length()),
                          inputs.left(m->keySeq().length()), usercount ));
            return to;
        }
    }
    return view->getBufferCursor();
}


// MOTIONS

YCursor YModeCommand::moveLeft(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    args.view->moveLeft(&viewCursor, args.count, false, args.standalone );
    return viewCursor.buffer();
}

YCursor YModeCommand::moveRight(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    args.view->moveRight(&viewCursor, args.count, false, args.standalone );
    return viewCursor.buffer();
}

YCursor YModeCommand::moveLeftWrap( const YMotionArgs & args )
{
    YViewCursor viewCursor = args.view->viewCursor();
    args.view->moveLeft(&viewCursor, args.count, true, args.standalone );
    return viewCursor.buffer();
}

YCursor YModeCommand::moveRightWrap( const YMotionArgs & args )
{
    YViewCursor viewCursor = args.view->viewCursor();
    args.view->moveRight(&viewCursor, args.count, true, args.standalone );
    return viewCursor.buffer();
}

YCursor YModeCommand::moveDown(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    if ( args.standalone )
        args.view->moveDown(&viewCursor, args.count, true );
    else { //LINEWISE
        //update starting point
        args.view->gotoxy( 0, viewCursor.bufferY(), false );
        // end point
        args.view->moveDown( &viewCursor, args.count + 1, false );
        args.view->moveToStartOfLine( &viewCursor, true );
    }
    return viewCursor.buffer();
}

YCursor YModeCommand::moveUp(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    if ( args.standalone )
        args.view->moveUp(&viewCursor, args.count, true );
    else { //LINEWISE
        //update starting point
        if ( viewCursor.bufferY() == args.view->myBuffer()->lineCount() - 1 )
            args.view->moveToEndOfLine( &viewCursor, false );
        else
            args.view->gotoxy( 0, viewCursor.bufferY() + 1, false );
        // end point
        args.view->moveUp( &viewCursor, args.count, false );
        args.view->gotoxy ( &viewCursor, 0, viewCursor.bufferY(), true );
    }
    return viewCursor.buffer();
}

void YModeCommand::scrollPageUp(const YCommandArgs &args)
{
    int line = args.view->getCurrentTop() - args.view->getLinesVisible();

    if (line < 0)
        line = 0;

    if (line != (int)args.view->getCurrentTop()) {
        args.view->alignViewBufferVertically( line );
    }
}

void YModeCommand::scrollLineUp(const YCommandArgs &args)
{
    int line = args.view->getCurrentTop() - 1;

    if (line < 0)
        line = 0;

    if (line != (int)args.view->getCurrentTop()) {
        args.view->alignViewBufferVertically( line );
    }
}

void YModeCommand::scrollPageDown(const YCommandArgs &args)
{
    int line = args.view->getCurrentTop() + args.view->getLinesVisible();
    YView *view = args.view;

    if (view->getLocalBooleanOption("wrap")) {
        YViewCursor temp = view->viewCursor();
        view->gotodxdy( &temp, view->getDrawCurrentLeft(), view->getDrawCurrentTop() + view->getLinesVisible() );

        line = temp.bufferY();
    }

    // don't scroll below the last line of the buffer
    if (line > view->myBuffer()->lineCount())
        line = view->myBuffer()->lineCount();

    if (line != view->getCurrentTop()) {
        view->alignViewBufferVertically( line );
    }
}

void YModeCommand::scrollLineDown(const YCommandArgs &args)
{
    int line = args.view->getCurrentTop() + args.view->getLinesVisible();
    YView *view = args.view;

    if (view->getLocalBooleanOption("wrap")) {
        YViewCursor temp = view->viewCursor();
        view->gotodxdy( &temp, view->getDrawCurrentLeft(), view->getDrawCurrentTop() + 1 );

        line = temp.bufferY();
    }

    // don't scroll below the last line of the buffer
    if (line > view->myBuffer()->lineCount())
        line = view->myBuffer()->lineCount();

    if (line != view->getCurrentTop()) {
        view->alignViewBufferVertically( line );
    }
}

YCursor YModeCommand::previousEmptyLine(const YMotionArgs &args)
{
    YCursor from = args.view->getBufferCursor();
    int start = from.y();
    int count = args.count > 0 ? args.count : 1;
    int counter = 0;
    while ( start >= 1 && counter != count) {
        if ( args.view->myBuffer()->textline(start - 1).isEmpty() ) {
            counter++;
        }
        start--;
    }

    YSession::self()->saveJumpPosition( QPoint(0, start));

    return YCursor(0, start);
}

YCursor YModeCommand::nextEmptyLine(const YMotionArgs &args)
{
    YCursor from = args.view->getBufferCursor();
    int start = from.y() + 1;
    int count = args.count > 0 ? args.count : 1;
    int counter = 0;
    while ( start < args.view->myBuffer()->lineCount() && counter != count ) {
        if ( args.view->myBuffer()->textline(start).isEmpty() ) {
            counter++;
        }
        start++;
    }

    YSession::self()->saveJumpPosition( QPoint(0, start - 1));

    return YCursor(0, start - 1);
}

YCursor YModeCommand::matchPair(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    bool found = false;
    YCursor pos = args.view->myBuffer()->action()->match( args.view, viewCursor.buffer(), &found );
    if ( found ) {
        if ( args.standalone ) {
            args.view->gotoxyAndStick( pos );
            YSession::self()->saveJumpPosition();
        }

        return pos;
    }

    return viewCursor.buffer();
}

YCursor YModeCommand::findNext(const YMotionArgs &args)
{
    YLineSearch* finder = args.view->myLineSearch();
    bool found;
    YCursor pos = finder->forward( args.arg, found, args.count );
    if ( found ) {
        if ( args.standalone )
            args.view->gotoxyAndStick( pos );
        return pos;
    }
    return args.view->getBufferCursor();
}

YCursor YModeCommand::findBeforeNext(const YMotionArgs &args)
{
    YLineSearch* finder = args.view->myLineSearch();
    bool found;
    YCursor pos = finder->forwardBefore( args.arg, found, args.count );
    if ( found ) {
        if ( args.standalone )
            args.view->gotoxyAndStick( pos );
        return pos;
    }
    return args.view->getBufferCursor();
}

YCursor YModeCommand::findPrevious(const YMotionArgs &args)
{
    YLineSearch* finder = args.view->myLineSearch();
    bool found;
    YCursor pos = finder->reverse( args.arg, found, args.count );
    if ( found ) {
        if ( args.standalone )
            args.view->gotoxyAndStick( pos );
        return pos;
    }
    return args.view->getBufferCursor();
}

YCursor YModeCommand::findAfterPrevious(const YMotionArgs &args)
{
    YLineSearch* finder = args.view->myLineSearch();
    bool found;
    YCursor pos = finder->reverseAfter( args.arg, found, args.count );
    if ( found ) {
        if ( args.standalone )
            args.view->gotoxyAndStick( pos );
        return pos;
    }
    return args.view->getBufferCursor();
}

YCursor YModeCommand::repeatFind(const YMotionArgs &args)
{
    YLineSearch* finder = args.view->myLineSearch();
    bool found;
    YCursor pos = finder->searchAgain( found, args.count );
    if ( found ) {
        if ( args.standalone )
            args.view->gotoxyAndStick( pos );
        return pos;
    }
    return args.view->getBufferCursor();
}

YCursor YModeCommand::gotoSOL(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    args.view->moveToStartOfLine(&viewCursor, args.standalone);
    return viewCursor.buffer();
}

YCursor YModeCommand::gotoEOL(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    args.view->moveToEndOfLine(&viewCursor, args.standalone);
    return viewCursor.buffer();
}

YCursor YModeCommand::gotoStartOfDocument(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    args.view->gotoLine(&viewCursor, 0, args.standalone);
    args.view->moveToStartOfLine(&viewCursor, args.standalone);
    return viewCursor.buffer();
}

YCursor YModeCommand::gotoEndOfDocument(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    args.view->gotoLastLine(&viewCursor, args.standalone);
    args.view->moveToEndOfLine(&viewCursor, args.standalone);
    return viewCursor.buffer();
}

YCursor YModeCommand::moveWordForward(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    QRegExp rex1("^\\w+\\s*"); //a word with boundaries
    QRegExp rex2("^[^\\w\\s]+\\s*"); //non-word chars with boundaries
    QRegExp ws("^\\s+"); //whitespace
    bool wrapped = false;

    while ( c < args.count ) { //for each word
        const QString& current = args.view->myBuffer()->textline( result.y() );
        //  if ( current.isNull() ) return false; //be safe ?

        int idx = rex1.indexIn( current, result.x(), QRegExp::CaretAtOffset );
        int len = rex1.matchedLength();
        if ( idx == 0 && wrapped )
            len = 0;
        if ( idx == -1 ) {
            idx = rex2.indexIn( current, result.x(), QRegExp::CaretAtOffset );
            len = rex2.matchedLength();
        }
        if ( idx == -1 ) {
            idx = ws.indexIn( current, result.x(), QRegExp::CaretAtOffset );
            len = ws.matchedLength();
        }
        if ( idx != -1 ) {
            dbg() << "Match at " << idx << " Matched length " << len << endl;
            c++; //one match
            result.setX( idx + len );
            if ( ( c < args.count || args.standalone )
                    && result.x() == current.length()
                    && result.y() < args.view->myBuffer()->lineCount() - 1) {
                result.setY(result.y() + 1);
                ws.indexIn(args.view->myBuffer()->textline( result.y() ));
                result.setX( qMax( ws.matchedLength(), 0 ));
            }
        } else {
            if ( result.y() >= args.view->myBuffer()->lineCount() - 1 ) {
                result.setX( current.length() );
                break;
            }
            result.setX(0);
            result.setY( result.y() + 1 );
            wrapped = true;
        }

    }
    if ( args.standalone )
        args.view->gotoxyAndStick( result );

    return result;
}


YCursor YModeCommand::moveSWordForward(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    QRegExp ws("\\s+"); //whitespace

    while ( c < args.count ) { //for each word
        const QString& current = args.view->myBuffer()->textline( result.y() );
        //  if ( current.isNull() ) return false; //be safe ?

        int idx = ws.indexIn( current, result.x(), QRegExp::CaretAtOffset );
        int len = ws.matchedLength();

        if ( idx != -1 ) {
            dbg() << "Match at " << idx << " Matched length " << len << endl;
            c++; //one match
            result.setX( idx + len );
            if ( ( c < args.count || args.standalone )
                    && result.x() == current.length()
                    && result.y() < args.view->myBuffer()->lineCount() - 1) {
                result.setY(result.y() + 1);
                ws.indexIn(args.view->myBuffer()->textline( result.y() ));
                result.setX( qMax( ws.matchedLength(), 0 ));
            }
        } else {
            if ( result.y() >= args.view->myBuffer()->lineCount() - 1 ) {
                result.setX( current.length() );
                break;
            }
            result.setX(0);
            result.setY( result.y() + 1 );
        }

    }
    if ( args.standalone )
        args.view->gotoxyAndStick( result );

    return result;
}


QString invertQString( const QString& from )
{
    QString res = "";
    for ( int i = from.length() - 1 ; i >= 0; i-- )
        res.append( from[ i ] );
    return res;
}

YCursor YModeCommand::moveWordBackward(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    QRegExp rex1("^(\\w+)\\s*"); //a word with boundaries
    QRegExp rex2("^([^\\w\\s]+)\\s*"); //non-word chars with boundaries
    QRegExp rex3("^\\s+([^\\w\\s$]+|\\w+)"); //whitespace
    bool wrapped = false;

    while ( c < args.count ) { //for each word
        const QString& current = invertQString( args.view->myBuffer()->textline( result.y() ) );
        int lineLength = current.length();
        int offset = lineLength - result.x();
        dbg() << current << " at " << offset << endl;


        int idx = rex1.indexIn( current, offset , QRegExp::CaretAtOffset );
        int len = rex1.cap( 1 ).length();
        dbg() << "rex1 : " << idx << "," << len << endl;
        if ( idx == -1 ) {
            idx = rex2.indexIn( current, offset, QRegExp::CaretAtOffset );
            len = rex2.cap( 1 ).length();
            dbg() << "rex2 : " << idx << "," << len << endl;
            if ( idx == -1 ) {
                idx = rex3.indexIn( current, offset, QRegExp::CaretAtOffset );
                len = rex3.matchedLength();
                dbg() << "rex3 : " << idx << "," << len << endl;
            }
        }
        if ( wrapped && lineLength == 0 ) {
            idx = 0;
            len = 0;
        }
        if ( idx != -1 ) {
            dbg() << "Match at " << idx << " = " << lineLength - idx << " Matched length " << len << endl;
            c++; //one match
            result.setX( lineLength - idx - len );
        } else {
            if ( result.y() == 0 ) break; //stop here
            dbg() << "Previous line " << result.y() - 1 << endl;
            const QString& ncurrent = args.view->myBuffer()->textline( result.y() - 1 );
            wrapped = true;
            result.setX( ncurrent.length() );
            result.setY( result.y() - 1 );
        }

    }

    if ( args.standalone )
        args.view->gotoxyAndStick( result );

    return result;
}


YCursor YModeCommand::moveSWordBackward(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    QRegExp rex1("([\\S]+)\\s*"); //

    while ( c < args.count ) { //for each word
        const QString& current = invertQString( args.view->myBuffer()->textline( result.y() ) );
        int lineLength = current.length();
        int offset = lineLength - result.x();
        dbg() << current << " at " << offset << endl;


        int idx = rex1.indexIn( current, offset , QRegExp::CaretAtOffset );
        int len = rex1.cap( 1 ).length();

        dbg() << "rex1 : " << idx << "," << len << endl;
        if ( idx != -1 ) {
            dbg() << "Match at " << idx << " = " << lineLength - idx << " Matched length " << len << endl;
            c++; //one match
            result.setX( lineLength - idx - len );
        } else {
            if ( result.y() == 0 ) break; //stop here
            dbg() << "Previous line " << result.y() - 1 << endl;
            const QString& ncurrent = args.view->myBuffer()->textline( result.y() - 1 );
            result.setX( ncurrent.length() );
            result.setY( result.y() - 1 );
        }

    }

    if ( args.standalone )
        args.view->gotoxyAndStick( result );

    return result;
}

YCursor YModeCommand::firstNonBlank(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    args.view->moveToFirstNonBlankOfLine(&viewCursor, args.standalone);
    return viewCursor.buffer();
}

YCursor YModeCommand::gotoMark( const YMotionArgs &args )
{
    YViewCursor viewCursor = args.view->viewCursor();
    YViewMarker *mark = args.view->myBuffer()->viewMarks();
    if ( mark->contains(args.arg))
        return mark->value(args.arg).mBuffer;
    else {
        dbg() << "WARNING! mark " << args.arg << " not found" << endl;
        return viewCursor.buffer();
    }
}

YCursor YModeCommand::firstNonBlankNextLine( const YMotionArgs &args )
{
    YViewCursor viewCursor = args.view->viewCursor();
    args.view->moveDown(&viewCursor, args.count, args.standalone );
    args.view->moveToFirstNonBlankOfLine( &viewCursor, args.standalone );
    return viewCursor.buffer();
}

YCursor YModeCommand::gotoLine(const YMotionArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    int line = 0;
    dbg() << "gotoLine " << args.cmd << "," << args.count << endl;
    if ( args.count > 0 ) line = args.count - 1;

    if ( args.cmd == "gg" || ( args.cmd == "G" && args.usercount ) ) {
        args.view->gotoLine( &viewCursor, line, args.standalone );
    } else {
        if ( args.cmd == "G" ) {
            args.view->gotoLastLine( &viewCursor, args.standalone );
            //args.view->moveToEndOfLine( &viewCursor, false );
        } else
            args.view->gotoLine( &viewCursor, 0, args.standalone );
    }
    if (YSession::getBooleanOption("startofline"))
        args.view->moveToFirstNonBlankOfLine();

    YSession::self()->saveJumpPosition();

    return viewCursor.buffer();
}

YCursor YModeCommand::searchWord(const YMotionArgs &args)
{
    YCursor from = args.view->getBufferCursor();

    QString word = args.view->myBuffer()->getWordAt( from );
    if ( ! word.isNull() ) {
        dbg() << "searchWord : " << word << endl;
        YCursor pos;
        bool found = true;
        bool moved = true;
        word = QRegExp::escape( word );
        if ( ! args.cmd.contains( 'g' ) ) {
            if ( word[ 0 ].isLetterOrNumber() || word[ 0 ] == '_' ) // \w
                word = "\\b" + word + "\\b";
            else
                word = word + "(?=[\\s\\w]|$)";
            //    word = "(?=^|[\\s\\w])" + word + "(?=[\\s\\w]|$)"; seems that positive lookahead cannot work together...
        }
        for ( int i = 0; found && i < args.count; i++ ) {
            if ( args.cmd.contains('*') ) {
                pos = YSession::self()->search()->forward( args.view->myBuffer(), word, &found, from );
            } else {
                pos = YSession::self()->search()->backward( args.view->myBuffer(), word, &found, from );
            }
            if ( found ) {
                from = pos;
                moved = true;
            }
        }
        if ( args.standalone && moved ) args.view->gotoxyAndStick( from );
    }
    return from;
}

YCursor YModeCommand::searchNext(const YMotionArgs &args)
{
    YCursor from = args.view->getBufferCursor();
    YCursor pos;
    bool found = true;
    bool moved = true;
    for ( int i = 0; found && i < args.count; i++ ) {
        pos = YSession::self()->search()->replayForward( args.view->myBuffer(), &found, from );
        if ( found ) {
            from = pos;
            moved = true;
        }
    }

    if ( args.standalone && moved ) {
        args.view->gotoxyAndStick( from );
        YSession::self()->saveJumpPosition();
    }

    return from;
}

YCursor YModeCommand::searchPrev(const YMotionArgs &args)
{
    YCursor from = args.view->getBufferCursor();
    YCursor pos;
    bool found = true;
    bool moved = false;
    for ( int i = 0; found && i < args.count; i++ ) {
        pos = YSession::self()->search()->replayBackward( args.view->myBuffer(), &found, from );
        if ( found ) {
            from = pos;
            moved = true;
        }
    }

    if ( args.standalone && moved ) {
        args.view->gotoxyAndStick( from );
        YSession::self()->saveJumpPosition();
    }

    return from;
}

// COMMANDS

void YModeCommand::execMotion( const YCommandArgs &args )
{
    const YMotion *m = dynamic_cast<const YMotion*>(args.cmd);
    YASSERT(m);
    YCursor to = (this->*(m->motionMethod()))(YMotionArgs(args.view, args.count, args.arg, args.cmd->keySeq(), args.usercount, true));
    //args.view->centerViewVertically( to.y() );
    args.view->gotoxy(to.x(), to.y());

}

YInterval YModeCommand::interval(const YCommandArgs& args)
{
    YCursor from( args.view->getBufferCursor() );
    YCursor to = move( args.view, args.arg, args.count, args.usercount );
    if ( from > to ) {
        YCursor tmp( from );
        from = to;
        to = tmp;
    }
    bool entireLines = ( args.arg.length() > 0 && args.arg[ 0 ] == QChar('\'') );
    if ( entireLines ) {
        from.setX( 0 );
        to.setX( 0 );
        to.setY( to.y() + 1 );
    }
    YInterval ret( from, YBound(to, true) );
    return ret;
}

void YModeCommand::appendAtEOL(const YCommandArgs &args)
{
    args.view->moveToEndOfLine();
    args.view->append();
}

void YModeCommand::append(const YCommandArgs &args)
{
    args.view->append();
}

void YModeCommand::change(const YCommandArgs &args)
{
    YInterval area = interval( args );
    YCursor cur = area.fromPos();

    dbg() << "YModeCommand::change " << area << endl;
    args.view->myBuffer()->action()->deleteArea(args.view, area, args.regs);

    if ( cur.y() >= args.view->myBuffer()->lineCount() ) {
        args.view->myBuffer()->action()->insertNewLine( args.view, 0, args.view->myBuffer()->lineCount() );
        args.view->modePool()->change( YMode::ModeInsert );
    } else {
        args.view->gotoxyAndStick( cur );
        // start insert mode, append if at EOL
        if ( cur.x() < args.view->myBuffer()->getLineLength( cur.y() ) )
            args.view->modePool()->change( YMode::ModeInsert );
        else
            args.view->append();
    }
    args.view->commitNextUndo();
}

void YModeCommand::changeLine(const YCommandArgs &args)
{
    int y = args.view->getBufferCursor().y();
    args.view->myBuffer()->action()->deleteLine(args.view, args.view->getBufferCursor(), args.count, args.regs);
    args.view->myBuffer()->action()->insertNewLine( args.view, 0, args.view->getBufferCursor().y() );
    args.view->modePool()->push( YMode::ModeInsert );
    args.view->gotoxy(0, y);
    //args.view->commitNextUndo();
}

void YModeCommand::changeToEOL(const YCommandArgs &args)
{
    YCursor to = move(args.view, "$", 1, false);
    args.view->myBuffer()->action()->deleteArea(args.view, args.view->getBufferCursor(), to, args.regs);
    args.view->append();
    //args.view->commitNextUndo();
}

void YModeCommand::deleteLine(const YCommandArgs &args)
{
    args.view->myBuffer()->action()->deleteLine(args.view, args.view->getBufferCursor(), args.count, args.regs);
    args.view->commitNextUndo();
}

void YModeCommand::deleteToEndOfLastLine(const YCommandArgs &args)
{
    dbg() << "YModeCommand::deleteToEndOfLastLine " << args.cmd;
    int toy = args.view->myBuffer()->lineCount();
    int tox = args.view->myBuffer()->getLineLength(toy);

    int fromy = args.view->getBufferCursor().y() > 0 ? args.view->getBufferCursor().y() - 1 : 0;
    int fromx = args.view->myBuffer()->getLineLength(fromy);
    //special case : the first line , we can't move up
    if (fromy == args.view->getBufferCursor().y()) {
        fromx = 0;
    }
    args.view->myBuffer()->action()->deleteArea(args.view, YCursor(fromx, fromy), YCursor(tox, toy), args.regs);

    YViewCursor viewCursor = args.view->viewCursor();
    args.view->gotoxy ( &viewCursor, 0, viewCursor.bufferY(), true );
    args.view->commitNextUndo();
}

void YModeCommand::deleteToEOL(const YCommandArgs &args)
{
    //in vim : 2d$ does not behave as d$d$, this is illogical ..., you cannot delete twice to end of line ...
    YCursor to = move(args.view, "$", 1, false);
    args.view->myBuffer()->action()->deleteArea(args.view, args.view->getBufferCursor(), to, args.regs);
    args.view->commitNextUndo();
}

void YModeCommand::insertAtSOL(const YCommandArgs &args)
{
    args.view->moveToFirstNonBlankOfLine();
    args.view->modePool()->push( YMode::ModeInsert );
}

void YModeCommand::insertAtCol1(const YCommandArgs &args)
{
    args.view->moveToStartOfLine();
    args.view->modePool()->push( YMode::ModeInsert );
}

void YModeCommand::gotoCommandMode(const YCommandArgs &args)
{
    args.view->modePool()->pop( YMode::ModeCommand );
}

void YModeCommand::gotoLineAtTop(const YCommandArgs &args)
{
    int line;

    line = ( args.usercount ) ? args.count - 1 : args.view->getBufferCursor().y();
    args.view->alignViewVertically( line );
    args.view->gotoLine( line );
    args.view->moveToFirstNonBlankOfLine();
}

void YModeCommand::gotoLineAtCenter(const YCommandArgs &args)
{
    int line;
    line = ( args.usercount ) ? args.count - 1 : args.view->getBufferCursor().y();
    args.view->centerViewVertically( line );
    args.view->gotoxy(args.view->viewCursor().bufferX(), line );
}

void YModeCommand::gotoLineAtBottom(const YCommandArgs &args)
{
    int line;
    //int linesFromCenter;

    line = ( args.usercount ) ? args.count - 1 : args.view->getBufferCursor().y();

    if ( args.view->getLocalBooleanOption("wrap") ) {
        // the textline could span several screen lines
        YViewCursor viewCursor = args.view->viewCursor();
        viewCursor.setBufferY(line);
        args.view->moveToEndOfLine(&viewCursor );

        args.view->bottomViewVertically( line + ( viewCursor.screenY() - line ) );
    } else
        args.view->bottomViewVertically( line );
    //}
    args.view->gotoLine( line );
    args.view->moveToFirstNonBlankOfLine();
}


void YModeCommand::gotoExMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeEx );
}
void YModeCommand::gotoInsertMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeInsert );
}
void YModeCommand::gotoReplaceMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeReplace );
}
void YModeCommand::gotoVisualMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeVisual );
}
void YModeCommand::gotoVisualLineMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeVisualLine );
}
void YModeCommand::gotoVisualBlockMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeVisualBlock );
}

void YModeCommand::insertLineAfter(const YCommandArgs &args)
{
    int y = args.view->getBufferCursor().y();
    YBuffer *mBuffer = args.view->myBuffer();
    mBuffer->action()->insertNewLine( args.view, mBuffer->textline( y ).length(), y );
    QStringList results = YSession::self()->eventCall("INDENT_ON_ENTER", args.view);
    if (results.count() > 0 ) {
        if (results[0].length() != 0) {
            mBuffer->action()->replaceLine( args.view, y + 1, results[0] + mBuffer->textline( y + 1 ).trimmed() );
            args.view->gotoxy(results[0].length(), y + 1);
        }
    }
    for ( int i = 1 ; i < args.count ; i++ ) {
        y = args.view->getBufferCursor().y();
        args.view->myBuffer()->action()->insertNewLine( args.view, 0, y );
        results = YSession::self()->eventCall("INDENT_ON_ENTER", args.view);
        if (results.count() > 0 ) {
            if (results[0].length() != 0) {
                mBuffer->action()->replaceLine( args.view, y + 1, results[0] + mBuffer->textline( y + 1 ).trimmed() );
                args.view->gotoxy(results[0].length(), y + 1);
            }
        }
    }
    args.view->modePool()->push( YMode::ModeInsert );
    args.view->moveToEndOfLine();
    //args.view->commitNextUndo();

}

void YModeCommand::insertLineBefore(const YCommandArgs &args)
{
    int y = args.view->getBufferCursor().y();
    for ( int i = 0 ; i < args.count ; i++ )
        args.view->myBuffer()->action()->insertNewLine( args.view, 0, y );
    args.view->moveUp();
    args.view->modePool()->push( YMode::ModeInsert );
    args.view->commitNextUndo();
}

void YModeCommand::joinLine(const YCommandArgs &args)
{
    for ( int i = 0; i < args.count; i++ )
        args.view->myBuffer()->action()->mergeNextLine( args.view, args.view->getBufferCursor().y(), true );
    args.view->commitNextUndo();
}

void YModeCommand::joinLineWithoutSpace(const YCommandArgs &args)
{
    for ( int i = 0; i < args.count; i++ )
        args.view->myBuffer()->action()->mergeNextLine( args.view, args.view->getBufferCursor().y(), false );
    args.view->commitNextUndo();
}

void YModeCommand::pasteAfter(const YCommandArgs &args)
{
    for ( int i = 0 ; i < args.count ; i++ )
        args.view->pasteContent( args.regs[ 0 ], true );
    args.view->commitNextUndo();
}

void YModeCommand::pasteBefore(const YCommandArgs &args)
{
    for ( int i = 0 ; i < args.count ; i++ )
        args.view->pasteContent( args.regs[ 0 ], false );
    args.view->commitNextUndo();
}

void YModeCommand::yankLine(const YCommandArgs &args)
{
    args.view->myBuffer()->action()->copyLine( args.view, args.view->getBufferCursor(), args.count, args.regs );
}

void YModeCommand::yankToEOL(const YCommandArgs &args)
{
    YCursor to = move(args.view, "$", 1, false);
    args.view->myBuffer()->action()->copyArea(args.view, args.view->getBufferCursor(), to, args.regs);
}

void YModeCommand::closeWithoutSaving(const YCommandArgs & /*args*/)
{
    YSession::self()->exitRequest( 0 );
}

void YModeCommand::saveAndClose(const YCommandArgs & /*args*/)
{
    YSession::self()->saveBufferExit();
}

void YModeCommand::searchBackwards(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeSearchBackward );
}

void YModeCommand::searchForwards(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeSearch );
}

void YModeCommand::del(const YCommandArgs &args)
{
    args.view->myBuffer()->action()->deleteArea( args.view, interval( args ), args.regs );
    args.view->commitNextUndo();
    args.view->modePool()->pop();
}

void YModeCommand::yank(const YCommandArgs &args)
{
    args.view->myBuffer()->action()->copyArea( args.view, interval( args ), args.regs );
    YCursor to = move( args.view, args.arg, args.count, args.usercount );
    args.view->gotoxyAndStick( to );
    args.view->modePool()->pop();
}

void YModeCommand::mark(const YCommandArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    args.view->myBuffer()->viewMarks()->insert( args.arg, viewCursor );
}

void YModeCommand::undo(const YCommandArgs &args)
{
    args.view->undo( args.count );
}

void YModeCommand::redo(const YCommandArgs &args)
{
    args.view->redo( args.count );
}

void YModeCommand::changeCase( const YCommandArgs &args )
{
    YCursor pos = args.view->getBufferCursor();
    const QString line = args.view->myBuffer()->textline( pos.y() );
    if ( ! line.isNull() ) {
        int length = line.length();
        int end = pos.x() + args.count;
        for ( ; pos.x() < length && pos.x() < end; pos.setX( pos.x() + 1 ) ) {
            QString ch = QString(line.at( pos.x() ));
            if ( ch != ch.toLower() )
                ch = ch.toLower();
            else
                ch = ch.toUpper();
            args.view->myBuffer()->action()->replaceChar( args.view, pos, ch );
        }
        args.view->commitNextUndo();
    }
}

void YModeCommand::lineToUpperCase( const YCommandArgs &args )
{
    YCursor pos = args.view->getBufferCursor();
    int i = 0;
    while ( i < args.count ) {
        const QString line = args.view->myBuffer()->textline( pos.y() + i );
        if ( ! line.isNull() ) {
            args.view->myBuffer()->action()->replaceLine( args.view, pos.y() + i , line.toUpper());
        }
        i++;
    }
    args.view->gotoxy( 0, pos.y() + i );
    args.view->commitNextUndo();
}

void YModeCommand::lineToLowerCase( const YCommandArgs &args )
{
    YCursor pos = args.view->getBufferCursor();
    int i = 0;
    while ( i < args.count ) {
        const QString line = args.view->myBuffer()->textline( pos.y() + i );
        if ( ! line.isNull() ) {
            args.view->myBuffer()->action()->replaceLine( args.view, pos.y() + i, line.toLower());
        }
        i++;
    }
    args.view->gotoxy( 0, pos.y() + i );
    args.view->commitNextUndo();
}

void YModeCommand::macro( const YCommandArgs &args )
{
    if ( args.view->isRecording() )
        args.view->stopRecordMacro();
    else
        args.view->recordMacro( args.regs );
    args.view->guiModeChanged();
}

void YModeCommand::replayMacro( const YCommandArgs &args )
{
    args.view->purgeInputBuffer();
    if ( args.view->isRecording()) {
        dbg() << "User asked to play a macro he is currently recording, forget it !" << endl;
        if ( args.view->registersRecorded() == args.regs )
            return ;
    }

    for ( int i = 0; i < args.count; i++ ) {
        for ( int ab = 0 ; ab < args.regs.size(); ++ab)
            YSession::self()->sendMultipleKeys(
                args.view,
                YSession::self()->getRegister(args.regs.at(ab))[0]
            );
    }
    args.view->commitNextUndo();
}

void YModeCommand::deleteChar( const YCommandArgs &args )
{
    YCursor to( args.view->getBufferCursor() );
    args.view->myBuffer()->action()->copyArea(args.view, args.view->getBufferCursor(), to, args.regs);
    args.view->myBuffer()->action()->deleteChar( args.view, args.view->getBufferCursor(), args.count );
    args.view->commitNextUndo();
}

void YModeCommand::deleteCharBackwards( const YCommandArgs &args )
{
    YCursor pos = args.view->getBufferCursor();
    int oldX = pos.x();
    int newX = oldX - args.count;
    if ( newX < 0 )
        newX = 0;
    int delCount = oldX - newX;
    if ( delCount == 0 )
        return ; // nothing to delete
    pos.setX( newX );
    args.view->myBuffer()->action()->deleteChar( args.view, pos, delCount );
    args.view->commitNextUndo();
}

void YModeCommand::substitute( const YCommandArgs &args )
{
    YCursor cur = args.view->getBufferCursor();

    args.view->myBuffer()->action()->deleteChar( args.view, args.view->getBufferCursor(), args.count );
    args.view->commitNextUndo();

    // start insert mode, append if at EOL
    if ( cur.x() != args.view->myBuffer()->getLineLength( cur.y() ) )
        args.view->modePool()->push( YMode::ModeInsert );
    else
        args.view->append();
}

void YModeCommand::redisplay( const YCommandArgs &args )
{
    args.view->recalcScreen();
}

void YModeCommand::replace( const YCommandArgs &args )
{
    YCursor pos = args.view->getBufferCursor();
    if (args.arg == "<ESC>") {
        return ;
    }
    args.view->myBuffer()->action()->replaceChar( args.view, pos, args.arg );
    args.view->gotoxy(pos.x(), pos.y(), true);
    args.view->updateStickyCol();
    args.view->commitNextUndo();
}

void YModeCommand::abort( const YCommandArgs& /*args*/)
{}

void YModeCommand::delkey( const YCommandArgs &args )
{
    args.view->myBuffer()->action()->deleteChar( args.view, args.view->getBufferCursor(), 1);
    args.view->commitNextUndo();
}

void YModeCommand::indent( const YCommandArgs& args )
{
    YInterval area = interval( args );
    int fromY = area.fromPos().y();
    int toY = area.toPos().y();
    if ( toY > fromY && area.to().opened() && area.toPos().x() == 0 )
        --toY;
    int maxY = args.view->myBuffer()->lineCount() - 1;
    if ( toY > maxY ) toY = maxY;
    int factor = ( args.cmd->keySeq()[0] == '<' ? -1 : 1 ) * args.count;
    for ( int l = fromY; l <= toY; l++ ) {
        args.view->myBuffer()->action()->indentLine( args.view, l, factor );
    }
    args.view->commitNextUndo();
    args.view->modePool()->pop();
}

void YModeCommand::redoLastCommand( const YCommandArgs & args )
{
    YView * view = args.view;
    execCommand( view, view->getLastInputBuffer() );
}

void YModeCommand::tagNext( const YCommandArgs & args )
{
    YView * view = args.view;
    YCursor from = view->getBufferCursor();
    QString word = view->myBuffer()->getWordAt( from );

    tagJumpTo(word);
}

void YModeCommand::tagPrev( const YCommandArgs & /*args*/ )
{
    tagPop();
}

void YModeCommand::undoJump( const YCommandArgs & /*args*/ )
{
    const YCursor cursor = YSession::self()->previousJumpPosition();
    YSession::self()->currentView()->centerViewVertically( cursor.y() );
    YSession::self()->currentView()->gotodxdy( cursor, true );
}

void YModeCommand::incrementNumber( const YCommandArgs& args )
{
    adjustNumber(args, args.count);
}

void YModeCommand::decrementNumber( const YCommandArgs& args )
{
    adjustNumber(args, -args.count);
}

void YModeCommand::adjustNumber( const YCommandArgs& args, int change )
{
    YCursor pos = args.view->getBufferCursor();
    //dbg() << "adjustNumber: pos: " << pos;
    QString line = args.view->myBuffer()->textline(pos.y());
    if ( !line[pos.x()].isDigit() ) {
        // not a number, unless we're on the minus of a number
        if ( line[pos.x()] == '-'
                && ( pos.x() + 1 < line.length() ) && line[pos.x() + 1].isDigit() ) {
            pos.setX(pos.x() + 1); // on the number
        } else {
            dbg() << "adjustNumber: no digit under cursor";
            return ;
        }
    }
    // find the boundaries of the number
    int begin;
    int end;
    for (begin = pos.x(); begin >= 0 && line[begin].isDigit(); --begin)
        ;
    if (begin < 0 || line[begin] != '-') {
        ++begin;
    }
    for (end = pos.x(); end < line.length() && line[end].isDigit(); ++end)
        ;
    --end;

    //dbg() << "adjustNumber: begin: " << begin << ", end: " << end;
    int number = line.mid(begin, end - begin + 1).toInt();
    dbg() << "adjustNumber: number:" << number;
    number += change;
    QString number_str = QString::number(number);
    // replace old integer string with the new one
    pos.setX(begin);
    args.view->myBuffer()->action()->replaceText(args.view, pos, end - begin + 1, number_str);
    // move onto the last digit
    pos.setX(begin + number_str.length() - 1);
    args.view->gotoxyAndStick(pos);
}
