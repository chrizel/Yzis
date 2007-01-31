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

#include "debug.h"

#include "portability.h"
#include "action.h"
#include "buffer.h"
#include "selection.h"
#include "session.h"
#include "view.h"
#include "viewcursor.h"

using namespace yzis;

YZModeVisual::YZModeVisual() : YZModeCommand() {
	mType = YZMode::MODE_VISUAL;
	mString = _( "[ Visual ]" );
	mSelMode = true;
	mMapMode = visual;
	commands.clear();
}
YZModeVisual::~YZModeVisual() {
	for ( int ab = 0 ; ab < commands.size(); ++ab)
		delete commands.at(ab);
	commands.clear();
}

void YZModeVisual::toClipboard( YZView* mView ) {
	YZInterval interval = mView->getSelectionPool()->visual()->bufferMap()[0];
	YZSession::self()->setClipboardText( mView->myBuffer()->getText( interval ).join( "\n" ), Clipboard::Selection );
}

YZInterval YZModeVisual::buildBufferInterval( YZView*, const YZViewCursor& from, const YZViewCursor& to ) {
	return YZInterval( from.buffer(), to.buffer() );
}
YZInterval YZModeVisual::buildScreenInterval( YZView*, const YZViewCursor& from, const YZViewCursor& to ) {
	return YZInterval( from.screen(), to.screen() );
}

void YZModeVisual::enter( YZView* mView ) {
	YZDoubleSelection* visual = mView->getSelectionPool()->visual();

	mView->setPaintAutoCommit( false );
	if ( ! visual->isEmpty() ) {
		mView->sendPaintEvent( visual->screenMap(), false );
		cursorMoved( mView );
	} else {
		YZViewCursor pos = mView->viewCursor();
		*mView->visualCursor() = pos;
		visual->addInterval( buildBufferInterval( mView, pos, pos ), buildScreenInterval( mView, pos, pos ) );
		mView->sendPaintEvent( visual->screenMap(), false );

		toClipboard( mView );
	}
	mView->commitPaintEvent();
	mView->emitSelectionChanged();
}
void YZModeVisual::leave( YZView* mView ) {
	YZDoubleSelection* visual = mView->getSelectionPool()->visual();
	mView->setPaintAutoCommit( false );
	mView->sendPaintEvent( visual->screenMap(), false );
	visual->clear();
	mView->commitPaintEvent();
	mView->emitSelectionChanged();
}
void YZModeVisual::cursorMoved( YZView* mView ) {
	YZDoubleSelection* visual = mView->getSelectionPool()->visual();

	YZViewCursor curPos = mView->viewCursor();
	YZViewCursor visPos = *mView->visualCursor();
	bool reverse = visPos.buffer() > curPos.buffer();
	YZInterval bufI = buildBufferInterval( mView, reverse ? curPos : visPos, reverse ? visPos : curPos );
	YZInterval scrI = buildScreenInterval( mView, reverse ? curPos : visPos, reverse ? visPos : curPos );
	YZInterval curI = visual->screenMap()[0];

	visual->clear();
	visual->addInterval( bufI, scrI );

	YZSelection tmp("tmp");
	tmp.addInterval( YZInterval( qMin( scrI.from(), curI.from() ), qMax( scrI.to(), curI.to() ) ) );
	tmp.delInterval( YZInterval( qMax( scrI.from(), curI.from() ), qMin( scrI.to(), curI.to() ) ) );
	mView->sendPaintEvent( tmp.map(), false );

	toClipboard( mView );
	mView->emitSelectionChanged();
}

void YZModeVisual::initCommandPool() {
	commands.append( new YZCommand("<ALT>:", (PoolMethod) &YZModeVisual::movetoExMode) );
	commands.append( new YZCommand("<ALT>i", (PoolMethod) &YZModeVisual::movetoInsertMode) );
	commands.append( new YZCommand("<CTRL>[", &YZModeCommand::gotoCommandMode) );
	commands.append( new YZCommand("<CTRL>l", &YZModeCommand::redisplay) );
	commands.append( new YZCommand("<DEL>", &YZModeCommand::del) );
	commands.append( new YZCommand("<ESC>", (PoolMethod) &YZModeVisual::escape) );
	commands.append( new YZCommand("<CTRL>c", (PoolMethod) &YZModeVisual::escape) );
	commands.append( new YZCommand(":", (PoolMethod) &YZModeVisual::gotoExMode ) );
	commands.append( new YZCommand("A", (PoolMethod) &YZModeVisual::commandAppend ) );
	commands.append( new YZCommand("D", (PoolMethod) &YZModeVisual::deleteWholeLines) );
	commands.append( new YZCommand("I", (PoolMethod) &YZModeVisual::commandInsert ) );
	commands.append( new YZCommand("S", (PoolMethod) &YZModeVisual::changeWholeLines) );
	commands.append( new YZCommand("u", (PoolMethod) &YZModeVisual::toLowerCase) );
	commands.append( new YZCommand("U", (PoolMethod) &YZModeVisual::toUpperCase) );
	commands.append( new YZCommand("X", (PoolMethod) &YZModeVisual::deleteWholeLines) );
	commands.append( new YZCommand("Y", (PoolMethod) &YZModeVisual::yankWholeLines ) );
	commands.append( new YZCommand("c", &YZModeCommand::change) );
	commands.append( new YZCommand("s", &YZModeCommand::change) );
	commands.append( new YZCommand("d", &YZModeCommand::del) );
	commands.append( new YZCommand("y", (PoolMethod) &YZModeVisual::yank) );
	commands.append( new YZCommand("x", &YZModeCommand::del) );
	commands.append( new YZCommand(">", &YZModeCommand::indent) );
	commands.append( new YZCommand("<", &YZModeCommand::indent) );

	commands.append( new YZCommand("<PDOWN>", &YZModeCommand::scrollPageDown) );
	commands.append( new YZCommand("<CTRL>f", &YZModeCommand::scrollPageDown) );
	commands.append( new YZCommand("<PUP>", &YZModeCommand::scrollPageUp) );
	commands.append( new YZCommand("<CTRL>b", &YZModeCommand::scrollPageUp) );
	initVisualCommandPool();
}
void YZModeVisual::initVisualCommandPool() {
	if ( type() == MODE_VISUAL ) 
		commands.append( new YZCommand("v", (PoolMethod) &YZModeVisual::escape) );
	else
		commands.append( new YZCommand("v", (PoolMethod) &YZModeVisual::translateToVisual) );
	if ( type() == MODE_VISUAL_LINE )
		commands.append( new YZCommand("V", (PoolMethod) &YZModeVisual::escape) );
	else
		commands.append( new YZCommand("V", (PoolMethod) &YZModeVisual::translateToVisualLine) );
	if ( type() == MODE_VISUAL_BLOCK ) 
		commands.append( new YZCommand("<CTRL>v", (PoolMethod) &YZModeVisual::escape) );
	else
		commands.append( new YZCommand("<CTRL>v", (PoolMethod) &YZModeVisual::translateToVisualBlock) );
}
void YZModeVisual::commandAppend( const YZCommandArgs& args ) {
	YZCursor pos = qMax( args.view->visualCursor()->buffer(), args.view->getBufferCursor() );
	args.view->modePool()->change( MODE_INSERT );
	args.view->gotoxy( pos.x(), pos.y() );
}
void YZModeVisual::commandInsert( const YZCommandArgs& args ) {
	YZCursor pos = qMin( args.view->visualCursor()->buffer(), args.view->getBufferCursor() );
	args.view->modePool()->change( MODE_INSERT );
	args.view->gotoxy( pos.x(), pos.y() );
}
void YZModeVisual::toLowerCase( const YZCommandArgs& args ) {
	YZInterval inter = interval( args );
	QStringList t = args.view->myBuffer()->getText( inter );
	QStringList lt;
	for( int i = 0; i < t.size(); i++ )
		lt << t[i].toLower();
	args.view->myBuffer()->action()->replaceArea( args.view, inter, lt );
	args.view->commitNextUndo();
}
void YZModeVisual::toUpperCase( const YZCommandArgs& args ) {
	YZInterval inter = interval( args );
	QStringList t = args.view->myBuffer()->getText( inter );
	QStringList lt;
	for( int i = 0; i < t.size(); i++ )
		lt << t[i].toUpper();
	args.view->myBuffer()->action()->replaceArea( args.view, inter, lt );
	args.view->commitNextUndo();
}
void YZModeVisual::changeWholeLines(const YZCommandArgs &args) {
	YZInterval i = interval(args);
	YZCursor from( 0, i.fromPos().y());
	YZCursor to( args.view->myBuffer()->getLineLength(i.toPos().y())-1, i.toPos().y());

	// delete selected lines and enter insert mode
	args.view->myBuffer()->action()->deleteArea( args.view, from, to, args.regs);
	args.view->modePool()->change( MODE_INSERT );
}
void YZModeVisual::deleteWholeLines(const YZCommandArgs &args) {
	YZInterval i = interval(args);
	unsigned int lines = i.toPos().y() - i.fromPos().y() + 1;
	if ( type() == MODE_VISUAL_LINE )
		--lines;

	// delete whole lines, even those who are only partially selected
	args.view->myBuffer()->action()->deleteLine(args.view, i.fromPos().y(), lines, args.regs);
	args.view->commitNextUndo();

	args.view->modePool()->pop();
}
void YZModeVisual::yankWholeLines(const YZCommandArgs &args) {
	YZCursor topLeft = args.view->getSelectionPool()->visual()->bufferMap()[0].fromPos();

	YZInterval i = interval(args);
	unsigned int lines = i.toPos().y() - i.fromPos().y() + 1;

	if (args.view->modePool()->currentType() == YZMode::MODE_VISUAL_LINE) {
		// visual line mode, we don't need to do anything special
		args.view->myBuffer()->action()->copyArea( args.view, i, args.regs);
	}
	else {
		// copy whole lines, even those who are only partially selected
		args.view->myBuffer()->action()->copyLine( args.view, i.fromPos(), lines, args.regs );
	}

	args.view->modePool()->pop();

	// move cursor to top left corner of selection (yes, this is correct behaviour :)
	args.view->gotoxy( topLeft.x(), topLeft.y(), true );
	args.view->updateStickyCol( );
}
void YZModeVisual::yank( const YZCommandArgs& args ) {
	YZCursor topLeft = interval( args ).fromPos();
	YZModeCommand::yank( args );
	args.view->gotoxyAndStick( topLeft.x(), topLeft.y() );
}
void YZModeVisual::translateToVisualLine( const YZCommandArgs& args ) {
	args.view->modePool()->change( MODE_VISUAL_LINE, false ); // just translate (don't leave current mode)
}
void YZModeVisual::translateToVisual( const YZCommandArgs& args ) {
	args.view->modePool()->change( MODE_VISUAL, false );
}
void YZModeVisual::translateToVisualBlock( const YZCommandArgs& args ) {
	args.view->modePool()->change( MODE_VISUAL_BLOCK, false );
}
void YZModeVisual::escape( const YZCommandArgs& args ) {
	args.view->modePool()->pop();
}
void YZModeVisual::gotoExMode( const YZCommandArgs& args ) {
	args.view->modePool()->push( MODE_EX );
	args.view->setCommandLineText( "'<,'>" );
}
void YZModeVisual::movetoExMode( const YZCommandArgs& args ) {
	args.view->modePool()->change( MODE_EX );
}
void YZModeVisual::movetoInsertMode( const YZCommandArgs& args ) {
	args.view->modePool()->change( MODE_INSERT );
}

YZInterval YZModeVisual::interval(const YZCommandArgs& args ) {
	return args.view->getSelectionPool()->visual()->bufferMap()[0];
}

/**
 * MODE VISUAL LINES
 */

YZModeVisualLine::YZModeVisualLine() : YZModeVisual() {
	mType = YZMode::MODE_VISUAL_LINE;
	mString = _("[ Visual Line ]");
}
YZModeVisualLine::~YZModeVisualLine() {
}

YZInterval YZModeVisualLine::buildBufferInterval( YZView* , const YZViewCursor& from, const YZViewCursor& to ) {
	YZBound bf( from.buffer() );
	YZBound bt( to.buffer(), true );
	bf.setPos( 0, from.bufferY() );
	bt.setPos( 0, to.bufferY() + 1 );
	return YZInterval( bf, bt );
}
YZInterval YZModeVisualLine::buildScreenInterval( YZView* mView, const YZViewCursor& from, const YZViewCursor& to ) {
	YZViewCursor pos = from;
	mView->gotoxy( &pos, 0, from.bufferY() );
	YZBound bf( pos.screen() );
	YZBound bt( pos.screen(), true );
	if ( to.bufferY() < mView->myBuffer()->lineCount() - 1 ) {
		mView->gotoxy( &pos, 0, to.bufferY() + 1 );
		bt.setPos( pos.screen() );
	} else {
		mView->gotoxy( &pos, qMax( 1, mView->myBuffer()->getLineLength( to.bufferY() ) ) - 1, to.bufferY() );
		bt.setPos( YZCursor( 0, pos.screenY() + 1 ) );
	}
	return YZInterval( bf, bt );
}

/**
 * MODE VISUAL BLOCK
 */

YZModeVisualBlock::YZModeVisualBlock() : YZModeVisual() {
	mType = YZMode::MODE_VISUAL_BLOCK;
	mString = _("[ Visual Block ]");
}
YZModeVisualBlock::~YZModeVisualBlock() {
}

void YZModeVisualBlock::cursorMoved( YZView* mView ) {
	mView->setPaintAutoCommit( false );

	YZDoubleSelection* visual = mView->getSelectionPool()->visual();
	YZSelection old = visual->screen();
	visual->clear();

	int fromLine = mView->visualCursor()->bufferY();
	int toLine = mView->getBufferCursor().y();
	int fromCol = (mView->visualCursor()->curLineHeight()-1)*mView->getColumnsVisible() + mView->visualCursor()->screenX();
	int toCol = (mView->viewCursor().curLineHeight()-1)*mView->getColumnsVisible() + mView->getCursor().x();

	YZViewCursor cur = *mView->visualCursor();
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
	yzDebug() << "visual block : from " << fromCol << "," << fromLine << " to " << toCol << "," << toLine << endl;
	YZInterval sI, bI;
	for ( int i = fromLine; i <= toLine; i++ ) {

		mView->gotodxy( &cur, fromCol, i );
		sI.setFromPos( YZCursor(fromCol,cur.screenY()) );
		bI.setFromPos( cur.buffer() );

		mView->gotodxy( &cur, toCol, i );
		if ( cur.screenX() < fromCol ) continue; // too far, skip this line
		sI.setTo( YZBound(YZCursor(toCol,cur.screenY())) );
		bI.setTo( YZBound(cur.buffer()) );

		visual->addInterval( bI, sI );
//		yzDebug() << "visual block>" << bI << ", " << sI << endl;
	}
	YZSelection diff = YZSelection::diff( old, visual->screen() );
	mView->sendPaintEvent( diff.map(), false );

	mView->commitPaintEvent();
	toClipboard( mView );
	mView->emitSelectionChanged();
}

