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

YModeVisual::YModeVisual() : YModeCommand()
{
    mType = YMode::ModeVisual;
    mString = _( "[ Visual ]" );
    mMapMode = MapVisual;
    commands.clear();
    mIsEditMode = true;
    mIsCmdLineMode = false;
    mIsSelMode = true;
}
YModeVisual::~YModeVisual()
{
    for ( int ab = 0 ; ab < commands.size(); ++ab)
        delete commands.at(ab);
    commands.clear();
}

void YModeVisual::toClipboard( YView* mView )
{
    YInterval interval = mView->getSelectionPool()->visual()->bufferMap()[0];
    YSession::self()->guiSetClipboardText( mView->myBuffer()->getText( interval ).join( "\n" ), Clipboard::Selection );
}

YInterval YModeVisual::buildBufferInterval( YView*, const YViewCursor& from, const YViewCursor& to )
{
    return YInterval( from.buffer(), to.buffer() );
}
YInterval YModeVisual::buildScreenInterval( YView*, const YViewCursor& from, const YViewCursor& to )
{
    return YInterval( from.screen(), to.screen() );
}

void YModeVisual::enter( YView* mView )
{
    YDoubleSelection* visual = mView->getSelectionPool()->visual();

    mView->setPaintAutoCommit( false );
    if ( ! visual->isEmpty() ) {
        mView->sendPaintEvent( visual->screenMap(), false );
        cursorMoved( mView );
    } else {
        YViewCursor pos = mView->viewCursor();
        *mView->visualCursor() = pos;
        visual->addInterval( buildBufferInterval( mView, pos, pos ), buildScreenInterval( mView, pos, pos ) );
        mView->sendPaintEvent( visual->screenMap(), false );

        toClipboard( mView );
    }
    mView->commitPaintEvent();
    mView->guiSelectionChanged();
}
void YModeVisual::leave( YView* mView )
{
    YDoubleSelection* visual = mView->getSelectionPool()->visual();
    mView->setPaintAutoCommit( false );
    mView->sendPaintEvent( visual->screenMap(), false );
    visual->clear();
    mView->commitPaintEvent();
    mView->guiSelectionChanged();
}
void YModeVisual::cursorMoved( YView* mView )
{
    YDoubleSelection* visual = mView->getSelectionPool()->visual();

    YViewCursor curPos = mView->viewCursor();
    YViewCursor visPos = *mView->visualCursor();
    bool reverse = visPos.buffer() > curPos.buffer();
    YInterval bufI = buildBufferInterval( mView, reverse ? curPos : visPos, reverse ? visPos : curPos );
    YInterval scrI = buildScreenInterval( mView, reverse ? curPos : visPos, reverse ? visPos : curPos );
    YInterval curI = visual->screenMap()[0];

    visual->clear();
    visual->addInterval( bufI, scrI );

    YSelection tmp("tmp");
    tmp.addInterval( YInterval( qMin( scrI.from(), curI.from() ), qMax( scrI.to(), curI.to() ) ) );
    tmp.delInterval( YInterval( qMax( scrI.from(), curI.from() ), qMin( scrI.to(), curI.to() ) ) );
    mView->sendPaintEvent( tmp.map(), false );

    toClipboard( mView );
    mView->guiSelectionChanged();
}

void YModeVisual::initCommandPool()
{
    commands.append( new YCommand("<ALT>:", (PoolMethod) &YModeVisual::movetoExMode) );
    commands.append( new YCommand("<ALT>i", (PoolMethod) &YModeVisual::movetoInsertMode) );
    commands.append( new YCommand("<CTRL>[", &YModeCommand::gotoCommandMode) );
    commands.append( new YCommand("<CTRL>l", &YModeCommand::redisplay) );
    commands.append( new YCommand("<DEL>", &YModeCommand::del) );
    commands.append( new YCommand("<ESC>", (PoolMethod) &YModeVisual::escape) );
    commands.append( new YCommand("<CTRL>c", (PoolMethod) &YModeVisual::escape) );
    commands.append( new YCommand(":", (PoolMethod) &YModeVisual::gotoExMode ) );
    commands.append( new YCommand("A", (PoolMethod) &YModeVisual::commandAppend ) );
    commands.append( new YCommand("D", (PoolMethod) &YModeVisual::deleteWholeLines) );
    commands.append( new YCommand("I", (PoolMethod) &YModeVisual::commandInsert ) );
    commands.append( new YCommand("S", (PoolMethod) &YModeVisual::changeWholeLines) );
    commands.append( new YCommand("u", (PoolMethod) &YModeVisual::toLowerCase) );
    commands.append( new YCommand("U", (PoolMethod) &YModeVisual::toUpperCase) );
    commands.append( new YCommand("X", (PoolMethod) &YModeVisual::deleteWholeLines) );
    commands.append( new YCommand("Y", (PoolMethod) &YModeVisual::yankWholeLines ) );
    commands.append( new YCommand("c", &YModeCommand::change) );
    commands.append( new YCommand("s", &YModeCommand::change) );
    commands.append( new YCommand("d", &YModeCommand::del) );
    commands.append( new YCommand("y", (PoolMethod) &YModeVisual::yank) );
    commands.append( new YCommand("x", &YModeCommand::del) );
    commands.append( new YCommand(">", &YModeCommand::indent) );
    commands.append( new YCommand("<", &YModeCommand::indent) );

    commands.append( new YCommand("<PDOWN>", &YModeCommand::scrollPageDown) );
    commands.append( new YCommand("<CTRL>f", &YModeCommand::scrollPageDown) );
    commands.append( new YCommand("<PUP>", &YModeCommand::scrollPageUp) );
    commands.append( new YCommand("<CTRL>b", &YModeCommand::scrollPageUp) );
    initVisualCommandPool();
}
void YModeVisual::initVisualCommandPool()
{
    if ( type() == ModeVisual )
        commands.append( new YCommand("v", (PoolMethod) &YModeVisual::escape) );
    else
        commands.append( new YCommand("v", (PoolMethod) &YModeVisual::translateToVisual) );
    if ( type() == ModeVisualLine )
        commands.append( new YCommand("V", (PoolMethod) &YModeVisual::escape) );
    else
        commands.append( new YCommand("V", (PoolMethod) &YModeVisual::translateToVisualLine) );
    if ( type() == ModeVisualBlock )
        commands.append( new YCommand("<CTRL>v", (PoolMethod) &YModeVisual::escape) );
    else
        commands.append( new YCommand("<CTRL>v", (PoolMethod) &YModeVisual::translateToVisualBlock) );
}
CmdState YModeVisual::commandAppend( const YCommandArgs& args )
{
    YCursor pos = qMax( args.view->visualCursor()->buffer(), args.view->getBufferCursor() );
    args.view->modePool()->change( ModeInsert );
    args.view->gotoxy( pos.x(), pos.y() );
    return CmdOk;
}
CmdState YModeVisual::commandInsert( const YCommandArgs& args )
{
    YCursor pos = qMin( args.view->visualCursor()->buffer(), args.view->getBufferCursor() );
    args.view->modePool()->change( ModeInsert );
    args.view->gotoxy( pos.x(), pos.y() );
    return CmdOk;
}
CmdState YModeVisual::toLowerCase( const YCommandArgs& args )
{
    bool stopped;
    YInterval inter = interval( args, &stopped );
    QStringList t = args.view->myBuffer()->getText( inter );
    QStringList lt;
    for ( int i = 0; i < t.size(); i++ )
        lt << t[i].toLower();
    args.view->myBuffer()->action()->replaceArea( args.view, inter, lt );
    args.view->commitNextUndo();
    return CmdOk;
}
CmdState YModeVisual::toUpperCase( const YCommandArgs& args )
{
    bool stopped;
    YInterval inter = interval( args, &stopped );
    QStringList t = args.view->myBuffer()->getText( inter );
    QStringList lt;
    for ( int i = 0; i < t.size(); i++ )
        lt << t[i].toUpper();
    args.view->myBuffer()->action()->replaceArea( args.view, inter, lt );
    args.view->commitNextUndo();
    return CmdOk;
}
CmdState YModeVisual::changeWholeLines(const YCommandArgs &args)
{
    bool stopped;
    YInterval i = interval(args, &stopped);
    YCursor from( 0, i.fromPos().y());
    YCursor to( args.view->myBuffer()->getLineLength(i.toPos().y()) - 1, i.toPos().y());

    // delete selected lines and enter insert mode
    args.view->myBuffer()->action()->deleteArea( args.view, from, to, args.regs);
    args.view->modePool()->change( ModeInsert );
    return CmdOk;
}
CmdState YModeVisual::deleteWholeLines(const YCommandArgs &args)
{
    bool stopped;
    YInterval i = interval(args, bool *stopped);
    unsigned int lines = i.toPos().y() - i.fromPos().y() + 1;
    if ( type() == ModeVisualLine )
        --lines;

    // delete whole lines, even those who are only partially selected
    args.view->myBuffer()->action()->deleteLine(args.view, i.fromPos().y(), lines, args.regs);
    args.view->commitNextUndo();

    args.view->modePool()->pop();
    return CmdOk;
}
CmdState YModeVisual::yankWholeLines(const YCommandArgs &args)
{
    YCursor topLeft = args.view->getSelectionPool()->visual()->bufferMap()[0].fromPos();

    bool stopped;
    YInterval i = interval(args, &stopped);
    unsigned int lines = i.toPos().y() - i.fromPos().y() + 1;

    if (args.view->modePool()->currentType() == YMode::ModeVisualLine) {
        // visual line mode, we don't need to do anything special
        args.view->myBuffer()->action()->copyArea( args.view, i, args.regs);
    } else {
        // copy whole lines, even those who are only partially selected
        args.view->myBuffer()->action()->copyLine( args.view, i.fromPos(), lines, args.regs );
    }

    args.view->modePool()->pop();

    // move cursor to top left corner of selection (yes, this is correct behaviour :)
    args.view->gotoxy( topLeft.x(), topLeft.y(), true );
    args.view->updateStickyCol( );
    return CmdOk;
}
CmdState YModeVisual::yank( const YCommandArgs& args )
{
    bool stopped;
    YCursor topLeft = interval( args, &stopped ).fromPos();
    YModeCommand::yank( args );
    args.view->gotoxyAndStick( topLeft.x(), topLeft.y() );
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

YInterval YModeVisual::interval(const YCommandArgs& args, bool *stopped )
{
    *stopped = false;
    return args.view->getSelectionPool()->visual()->bufferMap()[0];
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

YInterval YModeVisualLine::buildBufferInterval( YView* , const YViewCursor& from, const YViewCursor& to )
{
    YBound bf( from.buffer() );
    YBound bt( to.buffer(), true );
    bf.setPos( QPoint(0, from.bufferY()));
    bt.setPos( QPoint(0, to.bufferY() + 1));
    return YInterval( bf, bt );
}
YInterval YModeVisualLine::buildScreenInterval( YView* mView, const YViewCursor& from, const YViewCursor& to )
{
    YViewCursor pos = from;
    mView->gotoxy( &pos, 0, from.bufferY() );
    YBound bf( pos.screen() );
    YBound bt( pos.screen(), true );
    if ( to.bufferY() < mView->myBuffer()->lineCount() - 1 ) {
        mView->gotoxy( &pos, 0, to.bufferY() + 1 );
        bt.setPos( pos.screen() );
    } else {
        mView->gotoxy( &pos, qMax( 1, mView->myBuffer()->getLineLength( to.bufferY() ) ) - 1, to.bufferY() );
        bt.setPos( YCursor( 0, pos.screenY() + 1 ) );
    }
    return YInterval( bf, bt );
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
    mView->setPaintAutoCommit( false );

    YDoubleSelection* visual = mView->getSelectionPool()->visual();
    YSelection old = visual->screen();
    visual->clear();

    int fromLine = mView->visualCursor()->bufferY();
    int toLine = mView->getBufferCursor().y();
    int fromCol = (mView->visualCursor()->curLineHeight() - 1) * mView->getColumnsVisible() + mView->visualCursor()->screenX();
    int toCol = (mView->viewCursor().curLineHeight() - 1) * mView->getColumnsVisible() + mView->getCursor().x();

    YViewCursor cur = *mView->visualCursor();
    if ( fromCol > toCol ) {
        int tmp = toCol;
        toCol = fromCol;
        fromCol = tmp;
    }
    if ( fromLine > toLine ) {
        cur = mView->viewCursor();
        int tmp = toLine;
        toLine = fromLine;
        fromLine = tmp;
    }
    dbg() << "visual block : from " << fromCol << "," << fromLine << " to " << toCol << "," << toLine << endl;
    YInterval sI, bI;
    for ( int i = fromLine; i <= toLine; i++ ) {

        mView->gotodxy( &cur, fromCol, i );
        sI.setFromPos( YCursor(fromCol, cur.screenY()) );
        bI.setFromPos( cur.buffer() );

        mView->gotodxy( &cur, toCol, i );
        if ( cur.screenX() < fromCol ) continue; // too far, skip this line
        sI.setTo( YBound(YCursor(toCol, cur.screenY())) );
        bI.setTo( YBound(cur.buffer()) );

        visual->addInterval( bI, sI );
        //  dbg() << "visual block>" << bI << ", " << sI << endl;
    }
    YSelection diff = YSelection::diff( old, visual->screen() );
    mView->sendPaintEvent( diff.map(), false );

    mView->commitPaintEvent();
    toClipboard( mView );
    mView->guiSelectionChanged();
}

