/*  This file is part of the Yzis libraries
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>,
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

#include "mode_visual.h"
#include "mode_pool.h"
#include "debug.h"

#include "portability.h"
#include "action.h"
#include "buffer.h"
#include "selection.h"
#include "session.h"
#include "view.h"
#include "viewcursor.h"

#define dbg()    yzDebug("YModeVisual")
#define err()    yzError("YModeVisual")

using namespace yzis;

YModeVisual::YModeVisual() : YModeCommand(),
	mStartViewCursor()
{
    mType = YMode::ModeVisual;
    mString = _( "[ Visual ]" );
    mMapMode = MapVisual;
    commands.clear();
    mIsEditMode = true;
    mIsCmdLineMode = false;
    mIsSelMode = true;
	mSelectionType = yzis::SelectionVisual;
}
YModeVisual::~YModeVisual()
{
    for ( int ab = 0 ; ab < commands.size(); ++ab)
        delete commands.at(ab);
    commands.clear();
}

void YModeVisual::enter( YView* mView )
{
	if ( !mStartViewCursor.contains(mView) ) {
		mStartViewCursor[mView] = mView->viewCursor();
		mView->acquireLine(mStartViewCursor[mView].line());
    }
	cursorMoved(mView);
}
void YModeVisual::leave( YView* mView )
{
	YASSERT(mStartViewCursor.contains(mView));
	mView->setSelection(mSelectionType, YInterval());
	mView->releaseLine(mStartViewCursor[mView].line());
	mStartViewCursor.remove(mView);
}
void YModeVisual::cursorMoved( YView* mView )
{
	YInterval bufferSelection = buildBufferInterval(mView);
	YRawData selectedData = mView->setSelection(mSelectionType, bufferSelection);
	YSession::self()->guiSetClipboardText(selectedData.join("\n"), Clipboard::Selection);
}

YInterval YModeVisual::buildBufferInterval( YView* mView )
{
	YASSERT(mStartViewCursor.contains(mView));
	YViewCursor beginPos = mStartViewCursor[mView];
	YViewCursor endPos = mView->viewCursor();
	if ( beginPos > endPos ) {
		YViewCursor tmp = endPos;
		endPos = beginPos;
		beginPos = endPos;
	}
	return YInterval(beginPos.buffer(), endPos.buffer());
}

void YModeVisual::initCommandPool()
{
    commands.append( new YCommand(YKeySequence("<A-:>"), (PoolMethod) &YModeVisual::movetoExMode) );
    commands.append( new YCommand(YKeySequence("<A-i>"), (PoolMethod) &YModeVisual::movetoInsertMode) );
    commands.append( new YCommand(YKeySequence("<C-[>"), &YModeCommand::gotoCommandMode) );
    commands.append( new YCommand(YKeySequence("<C-l>"), &YModeCommand::redisplay) );
    commands.append( new YCommand(YKeySequence("<DEL>"), &YModeCommand::del) );
    commands.append( new YCommand(YKeySequence("<ESC>"), (PoolMethod) &YModeVisual::escape) );
    commands.append( new YCommand(YKeySequence("<C-c>"), (PoolMethod) &YModeVisual::escape) );
    commands.append( new YCommand(YKeySequence(":"), (PoolMethod) &YModeVisual::gotoExMode ) );
    commands.append( new YCommand(YKeySequence("A"), (PoolMethod) &YModeVisual::commandAppend ) );
    commands.append( new YCommand(YKeySequence("D"), (PoolMethod) &YModeVisual::deleteWholeLines) );
    commands.append( new YCommand(YKeySequence("I"), (PoolMethod) &YModeVisual::commandInsert ) );
    commands.append( new YCommand(YKeySequence("S"), (PoolMethod) &YModeVisual::changeWholeLines) );
    commands.append( new YCommand(YKeySequence("u"), (PoolMethod) &YModeVisual::toLowerCase) );
    commands.append( new YCommand(YKeySequence("U"), (PoolMethod) &YModeVisual::toUpperCase) );
    commands.append( new YCommand(YKeySequence("X"), (PoolMethod) &YModeVisual::deleteWholeLines) );
    commands.append( new YCommand(YKeySequence("Y"), (PoolMethod) &YModeVisual::yankWholeLines ) );
    commands.append( new YCommand(YKeySequence("c"), &YModeCommand::change) );
    commands.append( new YCommand(YKeySequence("s"), &YModeCommand::change) );
    commands.append( new YCommand(YKeySequence("d"), &YModeCommand::del) );
    commands.append( new YCommand(YKeySequence("y"), (PoolMethod) &YModeVisual::yank) );
    commands.append( new YCommand(YKeySequence("x"), &YModeCommand::del) );
    commands.append( new YCommand(YKeySequence(">"), &YModeCommand::indent) );
    commands.append( new YCommand(YKeySequence("<"), &YModeCommand::indent) );

    commands.append( new YMotion(YKeySequence("<C-f>"), &YModeCommand::scrollPageDown) );
    commands.append( new YMotion(YKeySequence("<C-b>"), &YModeCommand::scrollPageUp) );
    initVisualCommandPool();
}
void YModeVisual::initVisualCommandPool()
{
    if ( modeType() == ModeVisual )
        commands.append( new YCommand(YKeySequence("v"), (PoolMethod) &YModeVisual::escape) );
    else
        commands.append( new YCommand(YKeySequence("v"), (PoolMethod) &YModeVisual::translateToVisual) );
    if ( modeType() == ModeVisualLine )
        commands.append( new YCommand(YKeySequence("V"), (PoolMethod) &YModeVisual::escape) );
    else
        commands.append( new YCommand(YKeySequence("V"), (PoolMethod) &YModeVisual::translateToVisualLine) );
    if ( modeType() == ModeVisualBlock )
        commands.append( new YCommand(YKeySequence("<C-v>"), (PoolMethod) &YModeVisual::escape) );
    else
        commands.append( new YCommand(YKeySequence("<C-v>"), (PoolMethod) &YModeVisual::translateToVisualBlock) );
}
CmdState YModeVisual::commandAppend( const YCommandArgs& args )
{
    YCursor pos = qMax(mStartViewCursor[args.view], args.view->viewCursor()).buffer();
    args.view->modePool()->change( ModeInsert );
    args.view->gotoLinePosition(pos.y() , pos.x());
    return CmdOk;
}
CmdState YModeVisual::commandInsert( const YCommandArgs& args )
{
    YCursor pos = qMin(mStartViewCursor[args.view], args.view->viewCursor()).buffer();
    args.view->modePool()->change( ModeInsert );
    args.view->gotoLinePosition(pos.y() , pos.x());
    return CmdOk;
}
CmdState YModeVisual::toLowerCase( const YCommandArgs& args )
{
    CmdState state;
    YInterval inter = interval( args, &state );
    YRawData t = args.view->buffer()->dataRegion(inter);
    YRawData lt;
    for ( int i = 0; i < t.size(); i++ )
        lt << t[i].toLower();
	args.view->buffer()->action()->replaceArea(args.view, inter, lt);
	args.view->commitNextUndo();
    return CmdOk;
}
CmdState YModeVisual::toUpperCase( const YCommandArgs& args )
{
    CmdState state;
    YInterval inter = interval( args, &state );
    YRawData t = args.view->buffer()->dataRegion(inter);
    YRawData lt;
    for ( int i = 0; i < t.size(); i++ )
        lt << t[i].toUpper();
    args.view->buffer()->action()->replaceArea( args.view, inter, lt );
    args.view->commitNextUndo();
    return CmdOk;
}
CmdState YModeVisual::changeWholeLines(const YCommandArgs &args)
{
    CmdState state;
    YInterval i = interval(args, &state);
    YCursor from( 0, i.fromPos().y());
    YCursor to( args.view->buffer()->getLineLength(i.toPos().y()) - 1, i.toPos().y());

    // delete selected lines and enter insert mode
    args.view->buffer()->action()->deleteArea( args.view, from, to, args.regs);
    args.view->modePool()->change( ModeInsert );
    return CmdOk;
}
CmdState YModeVisual::deleteWholeLines(const YCommandArgs &args)
{
    CmdState state;
    YInterval i = interval(args, &state);
    unsigned int lines = i.toPos().y() - i.fromPos().y() + 1;
    if ( modeType() == ModeVisualLine )
        --lines;

    // delete whole lines, even those who are only partially selected
    args.view->buffer()->action()->deleteLine(args.view, i.fromPos().y(), lines, args.regs);
    args.view->commitNextUndo();

    args.view->modePool()->pop();
    return CmdOk;
}
CmdState YModeVisual::yankWholeLines(const YCommandArgs &args)
{
    CmdState state;
    YInterval i = interval(args, &state);
	YCursor topLeft = i.fromPos();
    unsigned int lines = i.toPos().y() - i.fromPos().y() + 1;

    if (args.view->modePool()->currentType() == YMode::ModeVisualLine) {
        // visual line mode, we don't need to do anything special
        args.view->buffer()->action()->copyArea( args.view, i, args.regs);
    } else {
        // copy whole lines, even those who are only partially selected
        args.view->buffer()->action()->copyLine( args.view, i.fromPos(), lines, args.regs );
    }

    args.view->modePool()->pop();

    // move cursor to top left corner of selection (yes, this is correct behaviour :)
    args.view->gotoLinePosition(topLeft.y() , topLeft.x());
    args.view->stickToColumn( );
    return CmdOk;
}
CmdState YModeVisual::yank( const YCommandArgs& args )
{
    CmdState state;
    YCursor topLeft = interval( args, &state ).fromPos();
    YModeCommand::yank( args );
    args.view->gotoLinePositionAndStick(topLeft.y() , topLeft.x());
    args.view->modePool()->pop();
    return CmdOk;
}
CmdState YModeVisual::translateToVisualLine( const YCommandArgs& args )
{
    args.view->modePool()->change( ModeVisualLine, false ); // just translate (don't leave current mode)
    return CmdOk;
}
CmdState YModeVisual::translateToVisual( const YCommandArgs& args )
{
    args.view->modePool()->change( ModeVisual, false );
    return CmdOk;
}
CmdState YModeVisual::translateToVisualBlock( const YCommandArgs& args )
{
    args.view->modePool()->change( ModeVisualBlock, false );
    return CmdOk;
}
CmdState YModeVisual::escape( const YCommandArgs& args )
{
    args.view->modePool()->pop();
    return CmdOk;
}
CmdState YModeVisual::gotoExMode( const YCommandArgs& args )
{
    args.view->modePool()->push( ModeEx );
    args.view->guiSetCommandLineText( "'<,'>" );
    return CmdOk;
}
CmdState YModeVisual::movetoExMode( const YCommandArgs& args )
{
    args.view->modePool()->change( ModeEx );
    return CmdOk;
}
CmdState YModeVisual::movetoInsertMode( const YCommandArgs& args )
{
    args.view->modePool()->change( ModeInsert );
    return CmdOk;
}

YInterval YModeVisual::interval(const YCommandArgs& args, CmdState *state )
{
    *state = CmdOk;
	return buildBufferInterval(args.view);
}

/**
 * MODE VISUAL LINES
 */

YModeVisualLine::YModeVisualLine() : YModeVisual()
{
    mType = YMode::ModeVisualLine;
    mString = _("[ Visual Line ]");
}
YModeVisualLine::~YModeVisualLine()
{}

YInterval YModeVisualLine::buildBufferInterval( YView* mView )
{
	YASSERT(mStartViewCursor.contains(mView));
	YViewCursor beginPos = mStartViewCursor[mView];
	YViewCursor endPos = mView->viewCursor();
	if ( beginPos > endPos ) {
		YViewCursor tmp = endPos;
		endPos = beginPos;
		beginPos = endPos;
	}
	return YInterval(YCursor(0,beginPos.line()), YBound(YCursor(0,endPos.line()+1), true));
}

/**
 * MODE VISUAL BLOCK
 */

YModeVisualBlock::YModeVisualBlock() : YModeVisual()
{
    mType = YMode::ModeVisualBlock;
    mString = _("[ Visual Block ]");
}
YModeVisualBlock::~YModeVisualBlock()
{}

void YModeVisualBlock::cursorMoved( YView* mView )
{
	//TODO
}

