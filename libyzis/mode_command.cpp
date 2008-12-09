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
#include "mode_pool.h"
#include "portability.h"
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

#include <QRegExp>


#define dbg()    yzDebug("YModeCommand")
#define err()    yzError("YModeCommand")

using namespace yzis;

YModeCommand::YModeCommand() : YMode()
{
    mType = ModeCommand;
    mString = _( "[ Awaiting Command ]" );
    commands.clear();
    motions.clear();
    mIsEditMode = false;
    mIsCmdLineMode = true;
    mIsSelMode = false;
}

YModeCommand::~YModeCommand()
{
    for ( int ab = 0 ; ab < commands.size(); ++ab)
        delete commands.at(ab);
    for ( int ab = 0; ab < motions.size(); ++ab )
        delete motions.at(ab);
    commands.clear();
    motions.clear();
}
void YModeCommand::init()
{
    initPool();
    initModifierKeys();
}

void YModeCommand::initPool()
{
    initGenericMotionPool();
    initMotionPool();
    initCommandPool();
}

void YModeCommand::initGenericMotionPool()
{
    // These are motions generic to most modes, basically those from special keys on the keyboard
    motions.append( new YMotion(YKeySequence("<HOME>"), &YModeCommand::gotoSOL, ArgNone) );
    motions.append( new YMotion(YKeySequence("<END>"), &YModeCommand::gotoEOL, ArgNone) );
    motions.append( new YMotion(YKeySequence("<C-HOME>"), &YModeCommand::gotoStartOfDocument, ArgNone) );
    motions.append( new YMotion(YKeySequence("<C-END>"), &YModeCommand::gotoEndOfDocument, ArgNone) );
    motions.append( new YMotion(YKeySequence("<PAGEUP>"), &YModeCommand::scrollPageUp, ArgNone) );
    motions.append( new YMotion(YKeySequence("<PAGEDOWN>"), &YModeCommand::scrollPageDown, ArgNone) );
    motions.append( new YMotion(YKeySequence("<LEFT>"), &YModeCommand::moveLeft, ArgNone) );
    motions.append( new YMotion(YKeySequence("<RIGHT>"), &YModeCommand::moveRight, ArgNone) );
    motions.append( new YMotion(YKeySequence("<UP>"),   &YModeCommand::moveUp,   ArgNone, MotionTypeLinewise) );
    motions.append( new YMotion(YKeySequence("<DOWN>"), &YModeCommand::moveDown, ArgNone, MotionTypeLinewise) );
    motions.append( new YMotion(YKeySequence("<S-LEFT>"), &YModeCommand::moveWordBackward, ArgNone) );
    motions.append( new YMotion(YKeySequence("<C-LEFT>"), &YModeCommand::moveSWordBackward, ArgNone) );
    motions.append( new YMotion(YKeySequence("<S-RIGHT>"), &YModeCommand::moveWordForward, ArgNone) );
    motions.append( new YMotion(YKeySequence("<C-RIGHT>"), &YModeCommand::moveSWordForward, ArgNone) );
    motions.append( new YMotion(YKeySequence("<S-UP>"), &YModeCommand::scrollPageUp, ArgNone) );
    motions.append( new YMotion(YKeySequence("<S-DOWN>"), &YModeCommand::scrollPageDown, ArgNone) );
}

void YModeCommand::initMotionPool()
{
    motions.append( new YMotion(YKeySequence("0"), &YModeCommand::gotoSOL, ArgNone) );
    motions.append( new YMotion(YKeySequence("$"), &YModeCommand::gotoEOL, ArgNone) );
    motions.append( new YMotion(YKeySequence("^"), &YModeCommand::firstNonBlank, ArgNone) );
    motions.append( new YMotion(YKeySequence("w"), &YModeCommand::moveWordForward, ArgNone) );
    motions.append( new YMotion(YKeySequence("W"), &YModeCommand::moveSWordForward, ArgNone) );
    motions.append( new YMotion(YKeySequence("b"), &YModeCommand::moveWordBackward, ArgNone) );
    motions.append( new YMotion(YKeySequence("B"), &YModeCommand::moveSWordBackward, ArgNone) );
    motions.append( new YMotion(YKeySequence("e"), &YModeCommand::moveWordEndForward, ArgNone, MotionTypeInclusive) );
    motions.append( new YMotion(YKeySequence("E"), &YModeCommand::moveSWordEndForward, ArgNone, MotionTypeInclusive) );
    motions.append( new YMotion(YKeySequence("ge"), &YModeCommand::moveWordEndBackward, ArgNone) );
    motions.append( new YMotion(YKeySequence("gE"), &YModeCommand::moveSWordEndBackward, ArgNone) );
    motions.append( new YMotion(YKeySequence("j"), &YModeCommand::moveDown, ArgNone, MotionTypeLinewise) );
    motions.append( new YMotion(YKeySequence("k"), &YModeCommand::moveUp,   ArgNone, MotionTypeLinewise) );
    motions.append( new YMotion(YKeySequence("h"), &YModeCommand::moveLeft, ArgNone) );
    motions.append( new YMotion(YKeySequence("l"), &YModeCommand::moveRight, ArgNone) );
    motions.append( new YMotion(YKeySequence("<BS>"), &YModeCommand::moveLeftWrap, ArgNone) );
    motions.append( new YMotion(YKeySequence("<SPACE>"), &YModeCommand::moveRightWrap, ArgNone) );
    motions.append( new YMotion(YKeySequence("f"), &YModeCommand::findNext, ArgChar, MotionTypeInclusive) );
    motions.append( new YMotion(YKeySequence("t"), &YModeCommand::findBeforeNext, ArgChar, MotionTypeInclusive) );
    motions.append( new YMotion(YKeySequence("F"), &YModeCommand::findPrevious, ArgChar, MotionTypeInclusive) );
    motions.append( new YMotion(YKeySequence("T"), &YModeCommand::findAfterPrevious, ArgChar, MotionTypeInclusive) );
    motions.append( new YMotion(YKeySequence(";"), &YModeCommand::repeatFind, ArgNone) );
    motions.append( new YMotion(YKeySequence("*"), &YModeCommand::searchWord, ArgNone) );
    motions.append( new YMotion(YKeySequence("g*"), &YModeCommand::searchWord, ArgNone) );
    motions.append( new YMotion(YKeySequence("#"), &YModeCommand::searchWord, ArgNone) );
    motions.append( new YMotion(YKeySequence("g#"), &YModeCommand::searchWord, ArgNone) );
    motions.append( new YMotion(YKeySequence("n"), &YModeCommand::searchNext, ArgNone) );
    motions.append( new YMotion(YKeySequence("N"), &YModeCommand::searchPrev, ArgNone) );
    motions.append( new YMotion(YKeySequence("%"), &YModeCommand::matchPair, ArgNone) );
    motions.append( new YMotion(YKeySequence("`"), &YModeCommand::gotoMark, ArgMark) );
    motions.append( new YMotion(YKeySequence("'"), &YModeCommand::gotoMark, ArgMark) );
    motions.append( new YMotion(YKeySequence("<ENTER>"), &YModeCommand::firstNonBlankNextLine, ArgNone) );
    motions.append( new YMotion(YKeySequence("+"), &YModeCommand::firstNonBlankNextLine, ArgNone) );
    motions.append( new YMotion(YKeySequence("-"), &YModeCommand::firstNonBlankPreviousLine, ArgNone) );
    motions.append( new YMotion(YKeySequence("gg"), &YModeCommand::gotoLine, ArgNone) );
    motions.append( new YMotion(YKeySequence("G"), &YModeCommand::gotoLine, ArgNone) );
    motions.append( new YMotion(YKeySequence("|"), &YModeCommand::gotoColumn, ArgNone) );
    motions.append( new YMotion(YKeySequence("}"), &YModeCommand::nextEmptyLine, ArgNone) );
    motions.append( new YMotion(YKeySequence("{"), &YModeCommand::previousEmptyLine, ArgNone) );
    motions.append( new YMotion(YKeySequence("<C-b>"), &YModeCommand::scrollPageUp) );
    motions.append( new YMotion(YKeySequence("<C-y>"), &YModeCommand::scrollLineUp) );
    motions.append( new YMotion(YKeySequence("<C-f>"), &YModeCommand::scrollPageDown) );
    motions.append( new YMotion(YKeySequence("<C-e>"), &YModeCommand::scrollLineDown) );
}

void YModeCommand::initCommandPool()
{
    commands.append( new YCommand(YKeySequence("I"), &YModeCommand::insertAtSOL) );
    commands.append( new YCommand(YKeySequence("gI"), &YModeCommand::insertAtCol1) );
    commands.append( new YCommand(YKeySequence("i"), &YModeCommand::gotoInsertMode) );
    commands.append( new YCommand(YKeySequence("<INSERT>"), &YModeCommand::gotoInsertMode) );
    commands.append( new YCommand(YKeySequence(":"), &YModeCommand::gotoExMode) );
    commands.append( new YCommand(YKeySequence("R"), &YModeCommand::gotoReplaceMode) );
    commands.append( new YCommand(YKeySequence("v"), &YModeCommand::gotoVisualMode) );
    commands.append( new YCommand(YKeySequence("V"), &YModeCommand::gotoVisualLineMode) );
    commands.append( new YCommand(YKeySequence("<C-v>"), &YModeCommand::gotoVisualBlockMode) );
    commands.append( new YCommand(YKeySequence("z<ENTER>"), &YModeCommand::gotoLineAtTop) );
    commands.append( new YCommand(YKeySequence("z+"), &YModeCommand::gotoLineAtTop) );
    commands.append( new YCommand(YKeySequence("z."), &YModeCommand::gotoLineAtCenter) );
    commands.append( new YCommand(YKeySequence("z-"), &YModeCommand::gotoLineAtBottom) );
    commands.append( new YCommand(YKeySequence("dd"), &YModeCommand::deleteLine) );
    commands.append( new YCommand(YKeySequence("dG"), &YModeCommand::deleteToEndOfLastLine) );
    commands.append( new YCommand(YKeySequence("d"), &YModeCommand::del, ArgMotion) );
    commands.append( new YCommand(YKeySequence("D"), &YModeCommand::deleteToEOL) );
    commands.append( new YCommand(YKeySequence("s"), &YModeCommand::substitute) );
    commands.append( new YCommand(YKeySequence("x"), &YModeCommand::deleteChar) );
    commands.append( new YCommand(YKeySequence("X"), &YModeCommand::deleteCharBackwards) );
    commands.append( new YCommand(YKeySequence("yy"), &YModeCommand::yankLine) );
    commands.append( new YCommand(YKeySequence("y"), &YModeCommand::yank, ArgMotion) );
    commands.append( new YCommand(YKeySequence("Y"), &YModeCommand::yankToEOL) );
    commands.append( new YCommand(YKeySequence("cc"), &YModeCommand::changeLine) );
    commands.append( new YCommand(YKeySequence("S"), &YModeCommand::changeLine) );
    commands.append( new YCommand(YKeySequence("c"), &YModeCommand::change, ArgMotion) );
    commands.append( new YCommand(YKeySequence("C"), &YModeCommand::changeToEOL) );
    commands.append( new YCommand(YKeySequence("p"), &YModeCommand::pasteAfter) );
    commands.append( new YCommand(YKeySequence("P"), &YModeCommand::pasteBefore) );
    commands.append( new YCommand(YKeySequence("o"), &YModeCommand::insertLineAfter) );
    commands.append( new YCommand(YKeySequence("O"), &YModeCommand::insertLineBefore) );
    commands.append( new YCommand(YKeySequence("a"), &YModeCommand::append) );
    commands.append( new YCommand(YKeySequence("A"), &YModeCommand::appendAtEOL) );
    commands.append( new YCommand(YKeySequence("J"), &YModeCommand::joinLine) );
    commands.append( new YCommand(YKeySequence("gJ"), &YModeCommand::joinLineWithoutSpace) );
    commands.append( new YCommand(YKeySequence("<LT>"), &YModeCommand::indent, ArgMotion ) );
    commands.append( new YCommand(YKeySequence("<LT><LT>"), &YModeCommand::indent ) );
    commands.append( new YCommand(YKeySequence("<GT>"), &YModeCommand::indent, ArgMotion ) );
    commands.append( new YCommand(YKeySequence("<GT><GT>"), &YModeCommand::indent ) );
    commands.append( new YCommand(YKeySequence("ZZ"), &YModeCommand::saveAndClose) );
    commands.append( new YCommand(YKeySequence("ZQ"), &YModeCommand::closeWithoutSaving) );
    commands.append( new YCommand(YKeySequence("/"), &YModeCommand::searchForwards) );
    commands.append( new YCommand(YKeySequence("?"), &YModeCommand::searchBackwards) );
    commands.append( new YCommand(YKeySequence("~"), &YModeCommand::changeCase) );
    commands.append( new YCommand(YKeySequence("m"), &YModeCommand::mark, ArgChar) );
    commands.append( new YCommand(YKeySequence("r"), &YModeCommand::replace, ArgChar) );
    commands.append( new YCommand(YKeySequence("u"), &YModeCommand::undo) );
    commands.append( new YCommand(YKeySequence("U"), &YModeCommand::redo) );
    commands.append( new YCommand(YKeySequence("<C-r>"), &YModeCommand::redo) );
    commands.append( new YCommand(YKeySequence("q"), &YModeCommand::macro) );
    commands.append( new YCommand(YKeySequence("@"), &YModeCommand::replayMacro) );
    commands.append( new YCommand(YKeySequence("<C-l>"), &YModeCommand::redisplay) );
    commands.append( new YCommand(YKeySequence("<C-[>"), &YModeCommand::gotoCommandMode) );
    commands.append( new YCommand(YKeySequence("<ESC>"), &YModeCommand::abort) );
    commands.append( new YCommand(YKeySequence("<C-c>"), &YModeCommand::abort) );
    commands.append( new YCommand(YKeySequence("<DELETE>"), &YModeCommand::delkey) );
    commands.append( new YCommand(YKeySequence("<A-:>"), &YModeCommand::gotoExMode) );
    commands.append( new YCommand(YKeySequence("gUU"), &YModeCommand::lineToUpperCase) );
    commands.append( new YCommand(YKeySequence("gUgU"), &YModeCommand::lineToUpperCase) );
    commands.append( new YCommand(YKeySequence("guu"), &YModeCommand::lineToLowerCase) );
    commands.append( new YCommand(YKeySequence("gugu"), &YModeCommand::lineToLowerCase) );
    commands.append( new YCommand(YKeySequence("."), &YModeCommand::redoLastCommand) );
    commands.append( new YCommand(YKeySequence("<C-]>"), &YModeCommand::tagNext) );
    commands.append( new YCommand(YKeySequence("<C-t>"), &YModeCommand::tagPrev) );
    commands.append( new YCommand(YKeySequence("<C-o>"), &YModeCommand::undoJump) );
    commands.append( new YCommand(YKeySequence("<C-a>"), &YModeCommand::incrementNumber) );
    commands.append( new YCommand(YKeySequence("<C-x>"), &YModeCommand::decrementNumber) );
}

void YModeCommand::initModifierKeys()
{
    mModifierKeys << "<SHIFT>" << "<CTRL>" << "<ALT>";
}

// Basic context-free grammar parsed by execCommand:
//
// Command := (count|register)* (MotionCmd | Cmd)
// MotionCmd := MOTION
// Cmd := CMD_WITH_CHAR CHAR | CMD_WITH_NONE | CMD_WITH_MARK CHAR
//        | CMD_WITH_MOTION ( (count) ? MotionCmd | 'a' textBlock | 'i' textBlock )
//
// Note: MotionCmd can always be preceeded by count, so will absorb that into parseMotion
CmdState YModeCommand::execCommand(YView *view, const YKeySequence &inputs, 
                                   YKeySequence::const_iterator &parsePos)
{
    dbg() << "ExecCommand( view, " << ", inputs='" << inputs.toString() << "')" << endl;
    CmdState result;
    int count = 1;
    bool hadCount = false;
    QList<QChar> regs;

    view->displayInfo("");
    if ( parsePos == inputs.end() )
        return CmdNotYetValid;

    // read in the register operations and the counts: (count|register)*
    while ( parsePos != inputs.end() ) {
        // Try to parse a count out of the input
        int tmpCount;
        tmpCount = inputs.parseUInt(parsePos);
        if ( tmpCount > 0 ) {
            count = tmpCount;
            hadCount = true;
        }
        else if ( *parsePos == Qt::Key_QuoteDbl ) {
            ++parsePos;
            if ( parsePos == inputs.end() )
                break;
            if ( parsePos->isUnicode() )
                regs << *parsePos;
            else
                return CmdError;
            ++parsePos;
        }
        else
            break;
    }
    
    //if regs is empty add the default register
    if ( regs.count() == 0 )
        regs << Qt::Key_QuoteDbl;

    if (parsePos == inputs.end())
        return CmdNotYetValid;

    // retrieve all the matching commands: (MotionCmd | Cmd)
    YKeySequence::const_iterator cmdPos = parsePos, motionResult=inputs.begin();
    MotionType motionType;
    YCommand *c = parseMotion( inputs, parsePos, count, motionType );
    // If it's not a motion, it's probably a regular command
    if ( c == NULL ) {
        motionResult = parsePos; // Save how successful motion was, in case it's a better match than command
        parsePos = cmdPos;
        c = parseCommand( inputs, parsePos );
    }
    
    // If still null, find out if we have incomplete command, or wrong command
    if ( c == NULL ) {
      if ( parsePos == inputs.end() || motionResult == inputs.end() )
            return CmdNotYetValid;
        else
            return CmdError;
    }

    // Actually execute the command, will parse rest of input as necessary
    foreach( YView *v, view->buffer()->views() )
        v->setPaintAutoCommit( false );

    result = (this->*(c->poolMethod()))(YCommandArgs(c, view, regs, count, hadCount, &inputs, &parsePos));
    
    foreach( YView *v, view->buffer()->views() )
        v->commitPaintEvent();    

    if ( c->arg() == ArgMark ) {
        YSession::self()->saveJumpPosition();
    }

    return result;
}

YCommand *YModeCommand::parseCommand( const YKeySequence &inputs, YKeySequence::const_iterator &initParsePos )
{
    dbg() << HERE() << endl;

    YKeySequence::const_iterator bestMatch = initParsePos, parsePos;
    QList<YCommand *> cmds;
    
    for ( QList<YCommand*>::const_iterator cs = commands.begin(); cs != commands.end(); ++cs ) {
        parsePos = initParsePos;
        if ( (*cs)->keySeq().match( parsePos, inputs.end() ) ) {
            // parsePos is updated by match
            if ( parsePos > bestMatch ) // Have a better match, so get rid of old ones
                cmds.clear();
            if ( parsePos >= bestMatch )
                cmds.append(*cs);
        }
        if ( parsePos > bestMatch )
          bestMatch = parsePos;
    }

    initParsePos = bestMatch;

    if ( cmds.isEmpty() )
        return NULL;
    
    YCommand *c;
    if ( cmds.count() != 1 ) {
        /* Assert precisely two matching commands, distinguished by argument */
//this is not sufficient, see the 'q' (record macro command), we need a q+ArgChar and a 'q' commands //mm //FIXME
        if ( MapVisual ) {
            if ( cmds[0]->arg() == ArgMotion) // Find the one with a motion arg
                c = cmds[0];
            else
                c = cmds[1];
        }
        else {
            if ( cmds[0]->arg() == ArgNone )
                c = cmds[0];
            else
                c = cmds[1];
        }
    }
    else
        c = cmds[0];
    return c;
}

// MotionCmd taken as (count)? MOTION_CMD
YMotion *YModeCommand::parseMotion( const YKeySequence &inputs, YKeySequence::const_iterator &initParsePos, int &count, MotionType &motionType)
{
    QList<YMotion *> mots;
    bool motionTypeSet = false;
    int tmpCount = inputs.parseUInt(initParsePos);
    if ( tmpCount != -1 )
        count *= tmpCount;

    motionType = MotionTypeExclusive;
    for(; initParsePos != inputs.end(); ++initParsePos) {
        if(*initParsePos == 'v')
            motionType = (motionType == MotionTypeExclusive) ? MotionTypeInclusive : MotionTypeExclusive;
        else if(*initParsePos == 'V')
            motionType = MotionTypeLinewise;
        else
            break;
        motionTypeSet = true;
    }

    YKeySequence::const_iterator bestMatch = initParsePos, parsePos;    
    for ( QList<YMotion*>::const_iterator ms = motions.begin(); ms != motions.end(); ++ms ) {
        parsePos = initParsePos;
        if ( (*ms)->keySeq().match( parsePos, inputs.end() ) ) {
            // parsePos is updated by match
            if ( parsePos > bestMatch )
                mots.clear();
            if ( parsePos >= bestMatch )
                mots.append(*ms);
        }
        if ( parsePos > bestMatch) {
          bestMatch = parsePos;
        }
    }

    initParsePos = bestMatch;
    
    if ( mots.isEmpty() )
        return NULL;

    if(!motionTypeSet)
        motionType = mots.first()->motionType();
    else if(motionType == MotionTypeInclusive || motionType == MotionTypeExclusive) // toggle inclusive/exclusive
        motionType = (mots.first()->motionType() == MotionTypeExclusive) ? MotionTypeInclusive : MotionTypeExclusive;
    else
        motionType = MotionTypeLinewise;

    return mots.first();
}

// MOTIONS

YCursor YModeCommand::moveLeft(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    *state = CmdOk;
    return args.view->viewCursorMoveHorizontal(-args.count).buffer();
}
YCursor YModeCommand::moveRight(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    *state = CmdOk;
    return args.view->viewCursorMoveHorizontal(args.count).buffer();
}

YCursor YModeCommand::moveLeftWrap( const YMotionArgs & args, CmdState *state, MotionStick* )
{
	bool stopped = false;
    YViewCursor ret = args.view->viewCursorMoveHorizontal(-args.count, true, &stopped);
    *state = stopped ? CmdStopped : CmdOk;
	return ret.buffer();
}
YCursor YModeCommand::moveRightWrap( const YMotionArgs & args, CmdState *state, MotionStick* )
{
	bool stopped = false;
    YViewCursor ret = args.view->viewCursorMoveHorizontal(args.count, true, &stopped);
    *state = stopped ? CmdStopped : CmdOk;
	return ret.buffer();
}

YCursor YModeCommand::moveDown(const YMotionArgs &args, CmdState *state, MotionStick* stick)
{
	if ( stick != NULL ) *stick = MotionNoStick;
    *state = CmdOk;
	return args.view->viewCursorMoveVertical(args.count).buffer();
}
YCursor YModeCommand::moveUp(const YMotionArgs &args, CmdState *state, MotionStick* stick)
{
	if ( stick != NULL ) *stick = MotionNoStick;
    *state = CmdOk;
	return args.view->viewCursorMoveVertical(-args.count).buffer();
}

YCursor YModeCommand::scrollPageUp(const YMotionArgs &args, CmdState *state, MotionStick* stick )
{
	if ( stick != NULL ) *stick = MotionNoStick;
    int line = qMin(args.view->topLine()+1, args.view->buffer()->lineCount()-1);
	args.view->scrollLineToBottom(line);
    *state = CmdOk;
	return args.view->viewCursorFromStickedLine(line).buffer();
}

YCursor YModeCommand::scrollLineUp(const YMotionArgs &args, CmdState *state, MotionStick* stick )
{
	if ( stick != NULL ) *stick = MotionNoStick;
    int line = qMax(args.view->currentLine()-1, 0);
	args.view->scrollLineToTop(line);
    *state = CmdOk;
	return args.view->viewCursorFromScreen().buffer();
}

YCursor YModeCommand::scrollPageDown(const YMotionArgs &args, CmdState *state, MotionStick* stick )
{
	if ( stick != NULL ) *stick = MotionNoStick;

    int line = qMax(args.view->bottomLine()-1, 0);
	args.view->scrollLineToTop(line);
    *state = CmdOk;
	return args.view->viewCursorFromStickedLine(line).buffer();
}

YCursor YModeCommand::scrollLineDown(const YMotionArgs &args, CmdState *state, MotionStick* stick )
{
	if ( stick != NULL ) *stick = MotionNoStick;
    int line = qMin(args.view->currentLine()+1, args.view->buffer()->lineCount()-1);
	args.view->scrollLineToTop(line);
    *state = CmdOk;
	return args.view->viewCursorFromScreen().buffer();
}

YCursor YModeCommand::previousEmptyLine(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YCursor from = args.view->getLinePositionCursor();
    int start = from.y();
    int count = args.count > 0 ? args.count : 1;
    int counter = 0;

    *state = CmdOk; // Always succeeds

    while ( start >= 1 && counter != count) {
        if ( args.view->buffer()->textline(start - 1).isEmpty() ) {
            counter++;
        }
        start--;
    }

    YSession::self()->saveJumpPosition( QPoint(0, start));

    return YCursor(0, start);
}

YCursor YModeCommand::nextEmptyLine(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YCursor from = args.view->getLinePositionCursor();
    int start = from.y() + 1;
    int count = args.count > 0 ? args.count : 1;
    int counter = 0;

    *state = CmdOk; // Always succeeds

    while ( start < args.view->buffer()->lineCount() && counter != count ) {
        if ( args.view->buffer()->textline(start).isEmpty() ) {
            counter++;
        }
        start++;
    }

    YSession::self()->saveJumpPosition( QPoint(0, start - 1));

    return YCursor(0, start - 1);
}

YCursor YModeCommand::matchPair(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YViewCursor viewCursor = args.view->viewCursor();
    bool found = false;
    YCursor pos = args.view->buffer()->action()->match( args.view, viewCursor.buffer(), &found );
    
    *state = CmdOk; // Always succeeds
    if ( found ) {
        if ( args.standalone ) {
            args.view->gotoLinePositionAndStick( pos );
            YSession::self()->saveJumpPosition();
        }

        return pos;
    }

    return viewCursor.buffer();
}

YCursor YModeCommand::findNext(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YLineSearch* finder = args.view->myLineSearch();
    bool found;

    YCursor pos = finder->forward( (*args.parsePos)->toString(), found, args.count );
    ++(*args.parsePos);

    if ( found ) {
        if ( args.standalone )
            args.view->gotoLinePositionAndStick( pos );
        *state = CmdOk;
        return pos;
    }

    *state = CmdStopped;
    return args.view->getLinePositionCursor();
}

YCursor YModeCommand::findBeforeNext(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YLineSearch* finder = args.view->myLineSearch();
    bool found;
    
    YCursor pos = finder->forwardBefore( (*args.parsePos)->toString(), found, args.count );
    ++(*args.parsePos);
    
    if ( found ) {
        if ( args.standalone )
            args.view->gotoLinePositionAndStick( pos );
        *state = CmdOk;
        return pos;
    }
    
    *state = CmdStopped;
    return args.view->getLinePositionCursor();
}

YCursor YModeCommand::findPrevious(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YLineSearch* finder = args.view->myLineSearch();
    bool found;
    YCursor pos = finder->reverse( (*args.parsePos)->toString(), found, args.count );
    ++(*args.parsePos);

    if ( found ) {
        if ( args.standalone )
            args.view->gotoLinePositionAndStick( pos );
        *state = CmdOk;
        return pos;
    }
    *state = CmdStopped;
    return args.view->getLinePositionCursor();
}

YCursor YModeCommand::findAfterPrevious(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YLineSearch* finder = args.view->myLineSearch();
    bool found;
    YCursor pos = finder->reverseAfter( (*args.parsePos)->toString(), found, args.count );
    ++(*args.parsePos);

    if ( found ) {
        if ( args.standalone )
            args.view->gotoLinePositionAndStick( pos );
        *state = CmdOk;
        return pos;
    }
    *state = CmdStopped;
    return args.view->getLinePositionCursor();
}

YCursor YModeCommand::repeatFind(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YLineSearch* finder = args.view->myLineSearch();
    bool found;
    YCursor pos = finder->searchAgain( found, args.count );
    if ( found ) {
        if ( args.standalone )
            args.view->gotoLinePositionAndStick( pos );
        *state = CmdOk;
        return pos;
    }
    *state = CmdStopped;
    return args.view->getLinePositionCursor();
}

YCursor YModeCommand::gotoSOL(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    *state = CmdOk;
	return YCursor(0, args.view->viewCursor().line());
}

YCursor YModeCommand::gotoEOL(const YMotionArgs &args, CmdState *state, MotionStick* stick)
{
    *state = CmdOk;
	if ( stick != NULL ) *stick = MotionStickEOL;
	return YCursor(args.view->buffer()->getLineLength(args.view->currentLine()), args.view->currentLine());
}

YCursor YModeCommand::gotoStartOfDocument(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    *state = CmdOk;
	return YCursor(0,0);
}

YCursor YModeCommand::gotoEndOfDocument(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    *state = CmdOk;
	int line = args.view->buffer()->lineCount() - 1;
	return YCursor(args.view->buffer()->getLineLength(line), line);
}

YCursor YModeCommand::moveWordForward(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    QRegExp rex1("^\\w+\\s*"); //a word with boundaries
    QRegExp rex2("^[^\\w\\s]+\\s*"); //non-word chars with boundaries
    QRegExp ws("^\\s+"); //whitespace
    bool wrapped = false;

    *state = CmdOk;

    while ( c < args.count ) { //for each word
        const QString& current = args.view->buffer()->textline( result.y() );
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
                    && result.y() < args.view->buffer()->lineCount() - 1) {
                result.setY(result.y() + 1);
                ws.indexIn(args.view->buffer()->textline( result.y() ));
                result.setX( qMax( ws.matchedLength(), 0 ));
            }
        } else {
            if ( result.y() >= args.view->buffer()->lineCount() - 1 ) {
                result.setX( current.length() );
                break;
            }
            result.setX(0);
            result.setY( result.y() + 1 );
            wrapped = true;
        }

    }
    if ( args.standalone )
        args.view->gotoLinePositionAndStick( result );

    return result;
}


YCursor YModeCommand::moveSWordForward(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    QRegExp ws("\\s+"); //whitespace

    *state = CmdOk;

    while ( c < args.count ) { //for each word
        const QString& current = args.view->buffer()->textline( result.y() );
        //  if ( current.isNull() ) return false; //be safe ?

        int idx = ws.indexIn( current, result.x(), QRegExp::CaretAtOffset );
        int len = ws.matchedLength();

        if ( idx != -1 ) {
            dbg() << "Match at " << idx << " Matched length " << len << endl;
            c++; //one match
            result.setX( idx + len );
            if ( ( c < args.count || args.standalone )
                    && result.x() == current.length()
                    && result.y() < args.view->buffer()->lineCount() - 1) {
                result.setY(result.y() + 1);
                ws.indexIn(args.view->buffer()->textline( result.y() ));
                result.setX( qMax( ws.matchedLength(), 0 ));
            }
        } else {
            if ( result.y() >= args.view->buffer()->lineCount() - 1 ) {
                result.setX( current.length() );
                break;
            }
            result.setX(0);
            result.setY( result.y() + 1 );
        }

    }
    if ( args.standalone )
        args.view->gotoLinePositionAndStick( result );

    return result;
}


QString invertQString( const QString& from )
{
    QString res = "";
    for ( int i = from.length() - 1 ; i >= 0; i-- )
        res.append( from[ i ] );
    return res;
}

YCursor YModeCommand::moveWordBackward(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    QRegExp rex1("^(\\w+)\\s*"); //a word with boundaries
    QRegExp rex2("^([^\\w\\s]+)\\s*"); //non-word chars with boundaries
    QRegExp rex3("^\\s+([^\\w\\s$]+|\\w+)"); //whitespace
    bool wrapped = false;

    *state = CmdOk;

    while ( c < args.count ) { //for each word
        const QString& current = invertQString( args.view->buffer()->textline( result.y() ) );
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
            const QString& ncurrent = args.view->buffer()->textline( result.y() - 1 );
            wrapped = true;
            result.setX( ncurrent.length() );
            result.setY( result.y() - 1 );
        }

    }

    if ( args.standalone )
        args.view->gotoLinePositionAndStick( result );

    return result;
}


YCursor YModeCommand::moveSWordBackward(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    QRegExp rex1("([\\S]+)\\s*"); //

    *state = CmdOk;

    while ( c < args.count ) { //for each word
        const QString& current = invertQString( args.view->buffer()->textline( result.y() ) );
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
            const QString& ncurrent = args.view->buffer()->textline( result.y() - 1 );
            result.setX( ncurrent.length() );
            result.setY( result.y() - 1 );
        }

    }

    if ( args.standalone )
        args.view->gotoLinePositionAndStick( result );

    return result;
}

YCursor YModeCommand::moveWordEndForward(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    QRegExp rex1("^\\s*\\w+"); //a word with leading whitespace
    QRegExp rex2("^\\s*[^\\w\\s]+"); //non-word chars with leading whitespace
    bool wrapped = false;

    *state = CmdOk;

    while ( c < args.count ) { //for each word end
        const QString& current = args.view->buffer()->textline( result.y() );
        //  if ( current.isNull() ) return false; //be safe ?
	if ( !wrapped && result.x() < current.length() )
	    result.setX( result.x() + 1 );
        int idx = rex1.indexIn( current, result.x(), QRegExp::CaretAtOffset );
        int len = rex1.matchedLength();
        if ( idx == -1 ) {
            idx = rex2.indexIn( current, result.x(), QRegExp::CaretAtOffset );
            len = rex2.matchedLength();
        } 

        if ( idx != -1 ) {
	    dbg() << "Match at " << idx << " Matched length " << len << endl;
	    c++; //one match
            wrapped = false;
	    result.setX( idx + len );
	    if(result.x() > 0 && result.x() < current.length())
		result.setX( result.x() - 1 );
        } else {
            if ( result.y() >= args.view->buffer()->lineCount() - 1 ) {
                result.setX( current.length() );
                break;
            }
            result.setX(0);
            result.setY( result.y() + 1 );
            wrapped = true;
        }
    }
    if ( args.standalone )
        args.view->gotoLinePositionAndStick( result );

    return result;
}

YCursor YModeCommand::moveSWordEndForward(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    QRegExp rex("^\\s*\\S+"); //an s-word with optional whitespace before
    bool wrapped = false;

    *state = CmdOk;

    while ( c < args.count ) { //for each word end
        const QString& current = args.view->buffer()->textline( result.y() );
        //  if ( current.isNull() ) return false; //be safe ?
	if ( !wrapped && result.x() < current.length() )
	    result.setX( result.x() + 1 );
        int idx = rex.indexIn( current, result.x(), QRegExp::CaretAtOffset );
        int len = rex.matchedLength();

        if ( idx != -1 ) {
	    dbg() << "Match at " << idx << " Matched length " << len << endl;
	    c++; //one match
            wrapped = false;
	    result.setX( idx + len );
	    if(result.x() > 0 && result.x() < current.length())
		result.setX( result.x() - 1 );
        } else {
            if ( result.y() >= args.view->buffer()->lineCount() - 1 ) {
                result.setX( current.length() );
                break;
            }
            result.setX(0);
            result.setY( result.y() + 1 );
            wrapped = true;
        }
    }
    if ( args.standalone )
        args.view->gotoLinePositionAndStick( result );

    return result;
}

YCursor YModeCommand::moveWordEndBackward(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    bool wrapped = false;
    QRegExp ws("^\\s+");
    QRegExp rex1("^\\w+\\s*");
    QRegExp rex2("^[^\\w\\s]+\\s*");

    *state = CmdOk;

    while ( c < args.count ) { //for each word
        const QString& current = invertQString( args.view->buffer()->textline( result.y() ) );
        int lineLength = current.length();
        int offset = lineLength - result.x() - 1;
        offset = qMax(offset, 0);
        dbg() << current << " at " << offset << endl;

        int idx, len;
        if ( wrapped && offset == 0 && current.length() > 0 && !current[0].isSpace() ) {
            idx = len = 0;
            dbg() << "word end at end of line" << endl;
        }
        else {
            idx = ws.indexIn( current, offset , QRegExp::CaretAtOffset );
            len = ws.matchedLength();
            if ( idx == -1 ) {
                idx = rex1.indexIn( current, offset , QRegExp::CaretAtOffset );
                len = rex1.matchedLength();
            }
            if ( idx == -1 ) {
                idx = rex2.indexIn( current, offset , QRegExp::CaretAtOffset );
                len = rex2.matchedLength();
            }
        }

        if ( idx != -1 && idx + len < lineLength ) {
            dbg() << "Match at " << idx << " = " << lineLength - idx << " Matched length " << len << endl;
            c++; //one match
            wrapped = false;
            result.setX( lineLength - idx - len );
	    if(result.x() > 0 && result.x() < current.length())
		result.setX( result.x() - 1 );
        } else {
            if ( result.y() == 0 ) {
                result.setX(0);
                break; //stop here
            }
            dbg() << "Previous line " << result.y() - 1 << endl;
            const QString& ncurrent = args.view->buffer()->textline( result.y() - 1 );
            wrapped = true;
            result.setX( ncurrent.length() );
            result.setY( result.y() - 1 );
        }

    }

    if ( args.standalone )
        args.view->gotoLinePositionAndStick( result );

    return result;
}

YCursor YModeCommand::moveSWordEndBackward(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YViewCursor viewCursor = args.view->viewCursor();
    YCursor result( viewCursor.buffer() );
    int c = 0;
    QRegExp rex1("(\\s+)\\S"); //whitespace

    *state = CmdOk;

    while ( c < args.count ) { //for each word
        const QString& current = invertQString( args.view->buffer()->textline( result.y() ) );
        int lineLength = current.length();
        int offset = lineLength - result.x();
        dbg() << current << " at " << offset << endl;

        int idx, len;
        if(offset == 0 && current.length() > 0 && !current[0].isSpace()) {
            idx = len = 0;
            dbg() << "word end at end of line" << endl;
        }
        else {
            idx = rex1.indexIn( current, offset , QRegExp::CaretAtOffset );
            len = rex1.cap(1).length();
            dbg() << "rex1 : " << idx << "," << len << endl;
        }

        if ( idx != -1 ) {
            dbg() << "Match at " << idx << " = " << lineLength - idx << " Matched length " << len << endl;
            c++; //one match
            result.setX( lineLength - idx - len );
	    if(result.x() > 0 && result.x() < current.length())
		result.setX( result.x() - 1 );

        } else {
            if ( result.y() == 0 ) {
                result.setX(0);
                break; //stop here
            }
            dbg() << "Previous line " << result.y() - 1 << endl;
            const QString& ncurrent = args.view->buffer()->textline( result.y() - 1 );
            result.setX( ncurrent.length() );
            result.setY( result.y() - 1 );
        }

    }

    if ( args.standalone )
        args.view->gotoLinePositionAndStick( result );

    return result;
}

YCursor YModeCommand::firstNonBlank(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    *state = CmdOk;
	int line = args.view->viewCursor().line();
	return YCursor(args.view->buffer()->firstNonBlankChar(line), line);
}

YCursor YModeCommand::gotoMark( const YMotionArgs &args, CmdState *state , MotionStick* )
{
    YCursor ret;
    YViewCursor viewCursor = args.view->viewCursor();
    YViewMarker *mark = args.view->buffer()->viewMarks();
    if ( mark->contains(QChar(*(*args.parsePos))) ) {
      *state = CmdOk;
        ret =  mark->value( QChar(*(*args.parsePos)) ).buffer();
    }
    else {
        dbg() << "WARNING! mark " << QChar(*(*args.parsePos)) << " not found" << endl;
        *state = CmdStopped;
        ret =  viewCursor.buffer();
    }
    ++(*args.parsePos);
    return ret;
}

YCursor YModeCommand::firstNonBlankNextLine( const YMotionArgs &args, CmdState *state , MotionStick* )
{
    *state = CmdOk;
	int line = qMax(args.view->buffer()->lineCount()-1, args.view->viewCursor().line()+1);
	return YCursor(args.view->buffer()->firstNonBlankChar(line), line);
}

YCursor YModeCommand::firstNonBlankPreviousLine( const YMotionArgs &args, CmdState *state , MotionStick* )
{
    *state = CmdOk;
	int line = qMin(0, args.view->viewCursor().line()-1);
	return YCursor(args.view->buffer()->firstNonBlankChar(line), line);
}

YCursor YModeCommand::gotoLine(const YMotionArgs &args, CmdState *state, MotionStick* stick)
{
    YViewCursor viewCursor = args.view->viewCursor();
    int line = 0;
    dbg() << "gotoLine," << args.count << endl;
    *state = CmdOk;

    if ( args.count > 0 ) line = args.count - 1;

	int dest_line = 0;
    if ( args.cmd == "gg" || ( args.cmd == "G" && args.usercount ) ) {
		dest_line = qMin(line, args.view->buffer()->lineCount()-1);
    } else if ( args.cmd == "G" ) {
		dest_line = args.view->buffer()->lineCount()-1;
    }
	if ( args.view->getLocalBooleanOption("startofline") ) {
		return YCursor(args.view->buffer()->firstNonBlankChar(dest_line), dest_line);
	} else {
		return args.view->viewCursorFromStickedLine(dest_line).buffer();
	}
}

YCursor YModeCommand::gotoColumn(const YMotionArgs &args, CmdState *state, MotionStick* stick)
{
    int columnToGo=0;
    int lineToGo;

    dbg() << "gotoColumn," << args.count << endl;
    *state = CmdOk;

    if (args.count >0) columnToGo = args.count -1;
    lineToGo = args.view->currentLine();    

    args.view->gotoLineColumnAndStick( lineToGo, columnToGo);
    
    return args.view->viewCursor().buffer();
}

YCursor YModeCommand::searchWord(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YCursor from = args.view->getLinePositionCursor();

    QString word = args.view->buffer()->getWordAt( from );
    *state = CmdOk;
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
                pos = YSession::self()->search()->forward( args.view->buffer(), word, &found, from );
            } else {
                pos = YSession::self()->search()->backward( args.view->buffer(), word, &found, from );
            }
            if ( found ) {
                from = pos;
                moved = true;
            }
        }
        if ( args.standalone && moved ) args.view->gotoLinePositionAndStick( from );
    }
    return from;
}

YCursor YModeCommand::searchNext(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YCursor from = args.view->getLinePositionCursor();
    YCursor pos;
    bool found = true;
    bool moved = true;
    *state = CmdStopped;
    for ( int i = 0; found && i < args.count; i++ ) {
        pos = YSession::self()->search()->replayForward( args.view->buffer(), &found, from );
        if ( found ) {
            from = pos;
            *state = CmdOk;
            moved = true;
        }
    }

    if ( args.standalone && moved ) {
        args.view->gotoLinePositionAndStick( from );
        YSession::self()->saveJumpPosition();
    }

    return from;
}

YCursor YModeCommand::searchPrev(const YMotionArgs &args, CmdState *state, MotionStick* )
{
    YCursor from = args.view->getLinePositionCursor();
    YCursor pos;
    bool found = true;
    bool moved = false;
    *state = CmdStopped;
    for ( int i = 0; found && i < args.count; i++ ) {
        pos = YSession::self()->search()->replayBackward( args.view->buffer(), &found, from );
        if ( found ) {
            *state = CmdOk;
            from = pos;
            moved = true;
        }
    }

    if ( args.standalone && moved ) {
        args.view->gotoLinePositionAndStick( from );
        YSession::self()->saveJumpPosition();
    }

    return from;
}

// COMMANDS

// To execute a motion, we find out where it goes, and go there.
CmdState YModeCommand::execMotion( const YCommandArgs &args )
{
    const YMotion *m = dynamic_cast<const YMotion*>(args.cmd);
    if ( ! m || ! m->argsPresent( *args.inputs, *args.parsePos ))
        return CmdOperatorPending;
    CmdState state;
	MotionStick stick = MotionStickColumn;
    YASSERT(m);
    YCursor dest = (this->*(m->motionMethod()))(YMotionArgs(args.view, args.count, args.inputs, 
                                                          args.parsePos, args.cmd->keySeq().toString(),
                                                          args.usercount, true), &state, &stick);
    args.view->gotoLinePosition(dest);
	switch ( stick ) {
		case MotionStickColumn:
			args.view->stickToColumn();
			break;
		case MotionStickEOL:
			args.view->stickToEOL();
			break;
		case MotionNoStick:
			break;
	}
    return state;
}

// Have Cmd requiring motion, so parse and find out where motion goes
YInterval YModeCommand::interval(const YCommandArgs& args, CmdState *state)
{
    // First things first, if we're at the end of input, operator is incomplete
    if ( *args.parsePos == args.inputs->end() ) {
        *state = CmdOperatorPending;
        return YInterval();
    }

    YKeySequence::const_iterator motPos = *args.parsePos;
    int count = args.count;
    MotionType motionType;
    YCursor from( args.view->getLinePositionCursor() );
    bool entireLines = ( *args.parsePos != args.inputs->end() 
                         && *(*args.parsePos) == YKey(Qt::Key_Apostrophe) );

    YMotion *m = parseMotion( *args.inputs, *args.parsePos, count, motionType );

    // Now we know whether we've got a valid motion
    // Decide whether a mismatch is due to incompleteness or error
    if ( !m && *args.parsePos == motPos ) {
        *state = CmdError;
        return YInterval();
    }
    else if ( !m || ! m->argsPresent(*args.inputs, *args.parsePos) ) {
        *state = CmdOperatorPending;
        return YInterval();
    }
    
    // We have a valid motion so lets try executing it
    YCursor to = (this->*(m->motionMethod()))(YMotionArgs(args.view, count, args.inputs, 
                                                          args.parsePos, args.cmd->keySeq().toString(),
                                                          args.usercount ), state, NULL);

    bool bound_open = true;
    switch(motionType) {
        case MotionTypeInclusive: bound_open  = false; break;
        case MotionTypeExclusive: bound_open  = true;  break;
        case MotionTypeLinewise:  entireLines = true;  break;
    }
    if ( from > to ) {
        YCursor tmp( from );
        from = to;
        to = tmp;
    }

    if ( entireLines ) {
        from.setX( 0 );
        to.setX( 0 );
        to.setY( to.y() + 1 );
        bound_open = true;
    }
    YInterval ret( from, YBound(to, bound_open) );
    return ret;
}

CmdState YModeCommand::appendAtEOL(const YCommandArgs &args)
{
	int line = args.view->viewCursor().line();
	args.view->gotoViewCursor(args.view->viewCursorFromLinePosition(line, args.view->buffer()->getLineLength(line)-1));
    args.view->append();
    return CmdOk;
}

CmdState YModeCommand::append(const YCommandArgs &args)
{
    args.view->append();
    return CmdOk;
}

CmdState YModeCommand::change(const YCommandArgs &args)
{
    CmdState state;
    YInterval area = interval( args, &state );
    YCursor cur = area.fromPos();

    if ( state != CmdOk )
        return state;
    dbg() << "YModeCommand::change " << area << endl;
    args.view->buffer()->action()->deleteArea(args.view, area, args.regs);

    if ( cur.y() >= args.view->buffer()->lineCount() ) {
        args.view->buffer()->action()->insertNewLine( args.view, 0, args.view->buffer()->lineCount() );
        args.view->modePool()->change( YMode::ModeInsert );
    } else {
        args.view->gotoLinePositionAndStick( cur );
        // start insert mode, append if at EOL
        if ( cur.x() < args.view->buffer()->getLineLength( cur.y() ) )
            args.view->modePool()->change( YMode::ModeInsert );
        else
            args.view->append();
    }
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::changeLine(const YCommandArgs &args)
{
    int y = args.view->getLinePositionCursor().y();
    args.view->buffer()->action()->deleteLine(args.view, args.view->getLinePositionCursor(), args.count, args.regs);
    if(!args.view->buffer()->isEmpty())
	args.view->buffer()->action()->insertNewLine( args.view, 0, args.view->getLinePositionCursor().y() );
    gotoInsertMode(args);
    args.view->gotoLinePosition(y, 0);
    //args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::changeToEOL(const YCommandArgs &args)
{
    YCursor to(args.view->buffer()->getLineLength(args.view->currentLine()), args.view->currentLine());
    args.view->buffer()->action()->deleteArea(args.view, args.view->getLinePositionCursor(), to, args.regs);
    args.view->append();
    //args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::deleteLine(const YCommandArgs &args)
{
    args.view->buffer()->action()->deleteLine(args.view, args.view->getLinePositionCursor(), args.count, args.regs);
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::deleteToEndOfLastLine(const YCommandArgs &args)
{
    dbg() << "YModeCommand::deleteToEndOfLastLine " << args.cmd;
    int toy = args.view->buffer()->lineCount() - 1;
    int tox = args.view->buffer()->getLineLength(toy);

    int fromy = args.view->getLinePositionCursor().y() > 0 ? args.view->getLinePositionCursor().y() - 1 : 0;
    int fromx = args.view->buffer()->getLineLength(fromy);
    //special case : the first line , we can't move up
    if (fromy == args.view->getLinePositionCursor().y()) {
        fromx = 0;
    }
    args.view->buffer()->action()->deleteArea(args.view, YCursor(fromx, fromy), YCursor(tox, toy), args.regs);

    YViewCursor viewCursor = args.view->viewCursor();
    args.view->gotoLinePosition(viewCursor.line() , 0);
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::deleteToEOL(const YCommandArgs &args)
{
    //in vim : 2d$ does not behave as d$d$, this is illogical ..., you cannot delete twice to end of line ...
    YCursor to(args.view->buffer()->getLineLength(args.view->currentLine()), args.view->currentLine());
    args.view->buffer()->action()->deleteArea(args.view, args.view->getLinePositionCursor(), to, args.regs);
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::insertAtSOL(const YCommandArgs &args)
{
	int line = args.view->viewCursor().line();
	args.view->gotoViewCursor(args.view->viewCursorFromLinePosition(line, args.view->buffer()->firstNonBlankChar(line)));
    return gotoInsertMode(args);
}

CmdState YModeCommand::insertAtCol1(const YCommandArgs &args)
{
	args.view->gotoViewCursor(args.view->viewCursorFromLinePosition(args.view->viewCursor().line(), 0));
    return gotoInsertMode(args);
}

CmdState YModeCommand::gotoCommandMode(const YCommandArgs &args)
{
    args.view->modePool()->pop( YMode::ModeCommand );
    return CmdOk;
}

CmdState YModeCommand::gotoLineAtTop(const YCommandArgs &args)
{
    int line;

    line = args.usercount ? args.count - 1 : args.view->currentLine();
    args.view->scrollLineToTop(line);
	args.view->gotoLinePositionAndStick(line, args.view->buffer()->firstNonBlankChar(line));
    return CmdOk;
}

CmdState YModeCommand::gotoLineAtCenter(const YCommandArgs &args)
{
    int line;
    line = args.usercount ? args.count - 1 : args.view->currentLine();
    args.view->scrollLineToCenter(line);
	args.view->gotoLinePositionAndStick(line, args.view->buffer()->firstNonBlankChar(line));
    return CmdOk;
}

CmdState YModeCommand::gotoLineAtBottom(const YCommandArgs &args)
{
    int line;
    //int linesFromCenter;

    line = args.usercount ? args.count - 1 : args.view->currentLine();
	args.view->scrollLineToBottom(line);
	args.view->gotoLinePositionAndStick(line, args.view->buffer()->firstNonBlankChar(line));
    return CmdOk;
}


CmdState YModeCommand::gotoExMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeEx );
    return CmdOk;
}
CmdState YModeCommand::gotoInsertMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeInsert );
    return CmdOk;
}
CmdState YModeCommand::gotoReplaceMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeReplace );
    return CmdOk;
}
CmdState YModeCommand::gotoVisualMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeVisual );
    return CmdOk;
}
CmdState YModeCommand::gotoVisualLineMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeVisualLine );
    return CmdOk;
}
CmdState YModeCommand::gotoVisualBlockMode(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeVisualBlock );
    return CmdOk;
}

CmdState YModeCommand::insertLineAfter(const YCommandArgs &args)
{
    int y = args.view->getLinePositionCursor().y();
    YBuffer *mBuffer = args.view->buffer();
    mBuffer->action()->insertNewLine( args.view, mBuffer->textline( y ).length(), y );
    QStringList results = YSession::self()->eventCall("INDENT_ON_ENTER", args.view);
    if (results.count() > 0 ) {
        if (results[0].length() != 0) {
            mBuffer->action()->replaceLine( args.view, y + 1, results[0] + mBuffer->textline( y + 1 ).trimmed() );
            args.view->gotoLinePosition(y + 1, results[0].length());
        }
    }
    for ( int i = 1 ; i < args.count ; i++ ) {
        y = args.view->getLinePositionCursor().y();
        args.view->buffer()->action()->insertNewLine( args.view, 0, y );
        results = YSession::self()->eventCall("INDENT_ON_ENTER", args.view);
        if (results.count() > 0 ) {
            if (results[0].length() != 0) {
                mBuffer->action()->replaceLine( args.view, y + 1, results[0] + mBuffer->textline( y + 1 ).trimmed() );
                args.view->gotoLinePosition(y + 1, results[0].length());
            }
        }
    }
    gotoInsertMode(args);
	int line = args.view->viewCursor().line();
	args.view->gotoViewCursor(args.view->viewCursorFromLinePosition(line, args.view->buffer()->getLineLength(line)));
    //args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::insertLineBefore(const YCommandArgs &args)
{
    int y = args.view->getLinePositionCursor().y();
    for ( int i = 0 ; i < args.count ; i++ )
        args.view->buffer()->action()->insertNewLine( args.view, 0, y );
    args.view->gotoViewCursor(args.view->viewCursorMoveVertical(-1));
    gotoInsertMode(args);
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::joinLine(const YCommandArgs &args)
{
    CmdState ret = CmdOk;
    
    for ( int i = 0; i < args.count; i++ ) {
        if ( args.view->getLinePositionCursor().y() == args.view->buffer()->lineCount()-1 ) {
            ret = CmdStopped;
            break;
        }        
        args.view->buffer()->action()->mergeNextLine( args.view, args.view->getLinePositionCursor().y(), true );
    }
    
    args.view->commitNextUndo();
    return ret;
}

CmdState YModeCommand::joinLineWithoutSpace(const YCommandArgs &args)
{
    CmdState ret = CmdOk;
    
    for ( int i = 0; i < args.count; i++ ) {
        if ( args.view->getLinePositionCursor().y() == args.view->buffer()->lineCount()-1 ) {
            ret = CmdStopped;
            break;
        }
        args.view->buffer()->action()->mergeNextLine( args.view, args.view->getLinePositionCursor().y(), false );
    }
    
    args.view->commitNextUndo();
    return ret;
}

CmdState YModeCommand::pasteAfter(const YCommandArgs &args)
{
    for ( int i = 0 ; i < args.count ; i++ )
                args.view->buffer()->action()->pasteContent( args.view, args.regs[ 0 ], true);
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::pasteBefore(const YCommandArgs &args)
{
    for ( int i = 0 ; i < args.count ; i++ )
                args.view->buffer()->action()->pasteContent( args.view, args.regs[ 0 ], false);
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::yankLine(const YCommandArgs &args)
{
    args.view->buffer()->action()->copyLine( args.view, args.view->getLinePositionCursor(), args.count, args.regs );
    return CmdOk;
}

CmdState YModeCommand::yankToEOL(const YCommandArgs &args)
{
	int line = args.view->viewCursor().line();
    YCursor to(args.view->buffer()->getLineLength(line) - 1, line);
    args.view->buffer()->action()->copyArea(args.view, args.view->viewCursor().buffer(), to, args.regs);
    return CmdOk;
}

CmdState YModeCommand::closeWithoutSaving(const YCommandArgs & /*args*/)
{
    YSession::self()->exitRequest( 0 );
    return CmdOk;
}

CmdState YModeCommand::saveAndClose(const YCommandArgs & /*args*/)
{
    YSession::self()->saveBufferExit();
    return CmdOk;
}

CmdState YModeCommand::searchBackwards(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeSearchBackward );
    return CmdOk;
}

CmdState YModeCommand::searchForwards(const YCommandArgs &args)
{
    args.view->modePool()->push( YMode::ModeSearch );
    return CmdOk;
}

CmdState YModeCommand::del(const YCommandArgs &args)
{
    CmdState state = CmdOk;

    YInterval area = interval( args, &state );
    
    if ( state != CmdOk )
        return state;

    args.view->buffer()->action()->deleteArea( args.view, area, args.regs );
    args.view->commitNextUndo();
    args.view->modePool()->pop();
    return CmdOk;
}

CmdState YModeCommand::yank(const YCommandArgs &args)
{
    CmdState state;
    YInterval area = interval( args, &state );
    
    if ( state != CmdOk )
        return state;
    
    args.view->buffer()->action()->copyArea( args.view, area, args.regs );
    args.view->gotoLinePositionAndStick( area.from().pos() );
    args.view->modePool()->pop();
    return CmdOk;
}

CmdState YModeCommand::mark(const YCommandArgs &args)
{
    YViewCursor viewCursor = args.view->viewCursor();
    if ( *args.parsePos == args.inputs->end() )
      return CmdOperatorPending;
    args.view->buffer()->viewMarks()->insert( (*args.parsePos)->toString(), viewCursor );
    ++(*args.parsePos);
    return CmdOk;
}

CmdState YModeCommand::undo(const YCommandArgs &args)
{
    args.view->undo( args.count );
    return CmdOk;
}

CmdState YModeCommand::redo(const YCommandArgs &args)
{
    args.view->redo( args.count );
    return CmdOk;
}

CmdState YModeCommand::changeCase( const YCommandArgs &args )
{
    YCursor pos = args.view->getLinePositionCursor();
    const QString line = args.view->buffer()->textline( pos.y() );
    if ( ! line.isNull() ) {
        int length = line.length();
        int end = pos.x() + args.count;
        for ( ; pos.x() < length && pos.x() < end; pos.setX( pos.x() + 1 ) ) {
            QString ch = QString(line.at( pos.x() ));
            if ( ch != ch.toLower() )
                ch = ch.toLower();
            else
                ch = ch.toUpper();
            args.view->buffer()->action()->replaceChar( args.view, pos, ch );
        }
        args.view->commitNextUndo();
    }
    return CmdOk;
}

CmdState YModeCommand::lineToUpperCase( const YCommandArgs &args )
{
    YCursor pos = args.view->getLinePositionCursor();
    int i = 0;
    while ( i < args.count ) {
        const QString line = args.view->buffer()->textline( pos.y() + i );
        if ( ! line.isNull() ) {
            args.view->buffer()->action()->replaceLine( args.view, pos.y() + i , line.toUpper());
        }
        i++;
    }
    args.view->gotoLinePosition(pos.y() + i , 0);
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::lineToLowerCase( const YCommandArgs &args )
{
    YCursor pos = args.view->getLinePositionCursor();
    int i = 0;
    while ( i < args.count ) {
        const QString line = args.view->buffer()->textline( pos.y() + i );
        if ( ! line.isNull() ) {
            args.view->buffer()->action()->replaceLine( args.view, pos.y() + i, line.toLower());
        }
        i++;
    }
    args.view->gotoLinePosition(pos.y() + i , 0);
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::macro( const YCommandArgs &args )
{
    if ( args.view->isRecording() )
        args.view->stopRecordMacro();
    else
        args.view->recordMacro( args.regs );
    args.view->updateMode();
    return CmdOk;
}

CmdState YModeCommand::replayMacro( const YCommandArgs &args )
{
    args.view->purgeInputBuffer();
    if ( args.view->isRecording()) {
        dbg() << "User asked to play a macro he is currently recording, forget it !" << endl;
        if ( args.view->registersRecorded() == args.regs )
            return CmdStopped;
    }

    for ( int i = 0; i < args.count; i++ ) {
        for ( int ab = 0 ; ab < args.regs.size(); ++ab) {
            YKeySequence inputs(YSession::self()->getRegister(args.regs.at(ab))[0]);
            YKeySequence::const_iterator parsePos = inputs.begin();
            if ( YSession::self()->sendMultipleKeys(args.view, inputs, parsePos) ) {
                args.view->commitNextUndo();
                return CmdStopped;
            }    
        }        
    }
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::deleteChar( const YCommandArgs &args )
{
    dbg() << HERE() << endl;
    YCursor to( args.view->getLinePositionCursor() );
    args.view->buffer()->action()->copyArea(args.view, args.view->getLinePositionCursor(), to, args.regs);
    args.view->buffer()->action()->deleteChar( args.view, args.view->getLinePositionCursor(), args.count );
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::deleteCharBackwards( const YCommandArgs &args )
{
    YCursor pos = args.view->getLinePositionCursor();
    int oldX = pos.x();
    int newX = oldX - args.count;
    if ( newX < 0 )
        newX = 0;
    int delCount = oldX - newX;
    if ( delCount == 0 )
        return CmdOk; // nothing to delete
    pos.setX( newX );
    args.view->buffer()->action()->deleteChar( args.view, pos, delCount );
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::substitute( const YCommandArgs &args )
{
    YCursor cur = args.view->getLinePositionCursor();

    args.view->buffer()->action()->deleteChar( args.view, args.view->getLinePositionCursor(), args.count );
    args.view->commitNextUndo();

    // start insert mode, append if at EOL
    if ( cur.x() != args.view->buffer()->getLineLength( cur.y() ) )
        args.view->modePool()->push( YMode::ModeInsert );
    else
        args.view->append();
    return CmdOk;
}

CmdState YModeCommand::redisplay( const YCommandArgs &args )
{
    args.view->recalcScreen();
    return CmdOk;
}

CmdState YModeCommand::replace( const YCommandArgs &args )
{
    YCursor pos = args.view->getLinePositionCursor();
    if ( *args.parsePos == args.inputs->end() )
        return CmdOperatorPending;
    if (*(*args.parsePos) == Qt::Key_Escape) {
        return CmdStopped;
    }
    if ( args.view->buffer()->action()->replaceChar( args.view, pos, (*args.parsePos)->toString() ) )
        return CmdStopped;
    args.view->gotoLinePosition(pos.y(), pos.x());
    args.view->stickToColumn();
    args.view->commitNextUndo();
    ++(*args.parsePos);
    return CmdOk;
}

CmdState YModeCommand::abort( const YCommandArgs& /*args*/)
{
    return CmdOk;
}

CmdState YModeCommand::delkey( const YCommandArgs &args )
{
    dbg() << HERE() << endl;
    if ( args.view->buffer()->action()->deleteChar( args.view, args.view->getLinePositionCursor(), 1) )
        return CmdStopped;
    args.view->commitNextUndo();
    return CmdOk;
}

CmdState YModeCommand::indent( const YCommandArgs& args )
{
    CmdState state;
    YInterval area;
    int factor = ( *((*args.parsePos)-1) == Qt::Key_Less? -1 : 1 ) * args.count;
    // First check how we got here: via a command with motion, or <</>> type
    // No easy way unfortunately
    if ( args.cmd->keySeq().count() == 2 ) {
        area = YInterval(args.view->getLinePositionCursor(), args.view->getLinePositionCursor());
        state = CmdOk;
    }
    else
        area = interval( args, &state );
    
    if ( state != CmdOk )
        return state;
    int fromY = area.fromPos().y();
    int toY = area.toPos().y();
    if ( toY > fromY && area.to().opened() && area.toPos().x() == 0 )
        --toY;
    int maxY = args.view->buffer()->lineCount() - 1;
    if ( toY > maxY ) toY = maxY;
    
    for ( int l = fromY; l <= toY; l++ ) {
        args.view->buffer()->action()->indentLine( args.view, l, factor );
    }
    args.view->commitNextUndo();
    args.view->modePool()->pop();
    return CmdOk;
}

CmdState YModeCommand::redoLastCommand( const YCommandArgs & args )
{
    YView * view = args.view;
    YKeySequence::const_iterator parsePos = view->getLastInputBuffer().begin();
    CmdState state = execCommand( view, view->getLastInputBuffer(), parsePos );
    if ( state == CmdNotYetValid )
      return CmdStopped;
    else
      return state;
}

CmdState YModeCommand::tagNext( const YCommandArgs & args )
{
    YView * view = args.view;
    YCursor from = view->getLinePositionCursor();
    QString word = view->buffer()->getWordAt( from );

    if ( tagJumpTo(word) )
        return CmdStopped;
    else
        return CmdOk;
}

CmdState YModeCommand::tagPrev( const YCommandArgs & /*args*/ )
{
    if ( tagPop() )
        return CmdStopped;
    else
        return CmdOk;
}

CmdState YModeCommand::undoJump( const YCommandArgs & /*args*/ )
{
    const YCursor cursor = YSession::self()->previousJumpPosition();
    YSession::self()->currentView()->scrollLineToCenter(cursor.line());
    YSession::self()->currentView()->gotoLinePosition(cursor);
    return CmdOk;
}

CmdState YModeCommand::incrementNumber( const YCommandArgs& args )
{
    return adjustNumber(args, args.count);
}

CmdState YModeCommand::decrementNumber( const YCommandArgs& args )
{
    return adjustNumber(args, -args.count);
}

CmdState YModeCommand::adjustNumber( const YCommandArgs& args, int change )
{
    YCursor pos = args.view->getLinePositionCursor();
    //dbg() << "adjustNumber: pos: " << pos;
    QString line = args.view->buffer()->textline(pos.y());
    if ( !line[pos.x()].isDigit() ) {
        // not a number, unless we're on the minus of a number
        if ( line[pos.x()] == '-'
                && ( pos.x() + 1 < line.length() ) && line[pos.x() + 1].isDigit() ) {
            pos.setX(pos.x() + 1); // on the number
        } else {
            dbg() << "adjustNumber: no digit under cursor";
            return CmdStopped;
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
    args.view->buffer()->action()->replaceText(args.view, pos, end - begin + 1, number_str);
    // move onto the last digit
    pos.setX(begin + number_str.length() - 1);
    args.view->gotoLinePositionAndStick(pos);
    return CmdOk;
}
