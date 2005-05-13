/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>,
 *  Copyright (C) 2005 Erlend Hamberg <ehamberg@online.no>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#include "portability.h"
#include "mode_visual.h"

#include "debug.h"

#include "action.h"
#include "buffer.h"
#include "selection.h"
#include "viewcursor.h"

#if QT_VERSION < 0x040000
#include <qkeysequence.h>
#include <qclipboard.h>
#else
#include <QX11Info>
#include <QApplication>
#include <QClipboard>
#endif


YZModeVisual::YZModeVisual() : YZModeCommand() {
	mType = YZMode::MODE_VISUAL;
	mString = _( "[ Visual ]" );
	mSelMode = true;
	mMapMode = visual;
	commands.clear();
#if QT_VERSION < 0x040000
	commands.setAutoDelete(true);
#endif
}
YZModeVisual::~YZModeVisual() {
#if QT_VERSION >= 0x040000
	for ( int ab = 0 ; ab < commands.size(); ++ab)
		delete commands.at(ab);
#endif
	commands.clear();
}

void YZModeVisual::toClipboard( YZView* mView ) {
	YZInterval interval = mView->getSelectionPool()->visual()->bufferMap()[0];
#ifndef YZIS_WIN32_MSVC
#if QT_VERSION < 0x040000
	if ( QPaintDevice::x11AppDisplay() )
#else
	if ( QX11Info::display() )
#endif
#endif
		QApplication::clipboard()->setText( mView->myBuffer()->getText( interval ).join( "\n" ), QClipboard::Selection );

}

YZInterval YZModeVisual::buildInterval( const YZCursor& from, const YZCursor& to ) {
	YZInterval ret( from, to );
	return ret;
}

void YZModeVisual::enter( YZView* mView ) {
	YZViewCursor* visualCursor = mView->visualCursor();
	YZDoubleSelection* visual = mView->getSelectionPool()->visual();

	mView->setPaintAutoCommit( false );
	if ( ! visual->isEmpty() ) {
		mView->sendPaintEvent( visual->screenMap(), false );
		cursorMoved( mView );
	} else {
		*visualCursor = mView->viewCursor();
		YZCursor buffer( *visualCursor->buffer() );
		YZCursor screen( *visualCursor->screen() );
		visual->addInterval( buildInterval(buffer,buffer), buildInterval(screen,screen) );
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

	YZInterval bufI = buildInterval(qMin(*mView->visualCursor()->buffer(),*mView->getBufferCursor()), 
					qMax(*mView->visualCursor()->buffer(),*mView->getBufferCursor()) );
	YZInterval scrI = buildInterval(qMin(*mView->visualCursor()->screen(),*mView->getCursor()), 
					qMax(*mView->visualCursor()->screen(),*mView->getCursor()) );
	YZInterval curI = visual->screenMap()[0];

	visual->clear();
	visual->addInterval( bufI, scrI );

	YZSelection tmp("tmp");
	if ( scrI.contains( curI ) ) {
		tmp.addInterval( scrI );
		tmp.delInterval( curI );
	} else {
		tmp.addInterval( curI );
		tmp.delInterval( scrI );
	}
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
	YZCursor pos = qMax( *args.view->visualCursor()->buffer(), *args.view->getBufferCursor() );
	args.view->modePool()->change( MODE_INSERT );
	args.view->gotoxy( pos.x(), pos.y() );
}
void YZModeVisual::commandInsert( const YZCommandArgs& args ) {
	YZCursor pos = qMin( *args.view->visualCursor()->buffer(), *args.view->getBufferCursor() );
	args.view->modePool()->change( MODE_INSERT );
	args.view->gotoxy( pos.x(), pos.y() );
}
void YZModeVisual::toLowerCase( const YZCommandArgs& args ) {
	YZInterval inter = interval( args );
	QStringList t = args.view->myBuffer()->getText( inter );
	QStringList lt;
	for( unsigned int i = 0; i < t.size(); i++ )
#if QT_VERSION < 0x040000
		lt << t[i].lower();
#else
		lt << t[i].toLower();
#endif
	args.view->myBuffer()->action()->replaceArea( args.view, inter, lt );
	args.view->commitNextUndo();
}
void YZModeVisual::toUpperCase( const YZCommandArgs& args ) {
	YZInterval inter = interval( args );
	QStringList t = args.view->myBuffer()->getText( inter );
	QStringList lt;
	for( unsigned int i = 0; i < t.size(); i++ )
#if QT_VERSION < 0x040000
		lt << t[i].upper();
#else
		lt << t[i].toUpper();
#endif
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

YZInterval YZModeVisualLine::buildInterval( const YZCursor& from, const YZCursor& to ) {
	YZBound bf( from );
	YZBound bt( to, true );
	bf.setPos( 0, from.y() );
	bt.setPos( 0, to.y() + 1 );
	YZInterval ret( bf, bt );
	return ret;
}

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
//	mView->sendPaintEvent( visual->screenMap(), false );
	visual->clear();

	unsigned int fromLine = mView->visualCursor()->bufferY();
	unsigned int toLine = mView->getBufferCursor()->y();
	unsigned int fromCol = (mView->visualCursor()->curLineHeight()-1)*mView->getColumnsVisible() + mView->visualCursor()->screenX();
	unsigned int toCol = (mView->viewCursor().curLineHeight()-1)*mView->getColumnsVisible() + mView->getCursor()->x();

	YZViewCursor cur = *mView->visualCursor();
	if ( fromCol > toCol ) {
		unsigned int tmp = toCol;
		toCol = fromCol;
		fromCol = tmp;
	}
	if ( fromLine > toLine ) {
		cur = mView->viewCursor();
		unsigned int tmp = toLine;
		toLine = fromLine;
		fromLine = tmp;
	}
	YZInterval sI, bI;
	for ( unsigned int i = fromLine; i <= toLine; i++ ) {
		mView->gotodxy( &cur, fromCol, i );
		if ( cur.screenX() < fromCol ) continue; // XXX handling tab is not easy
		sI.setFromPos( cur.screen() );
		bI.setFromPos( cur.buffer() );
		mView->gotodxy( &cur, toCol, i );
//		if ( cur.screenX() > toCol ) continue; // XXX handling tab is not easy
		sI.setTo( YZBound(cur.screen()) );
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

void YZModeVisualBlock::toClipboard( YZView* mView ) {
	YZInterval interval = mView->getSelectionPool()->visual()->bufferMap()[0];
#ifndef WIN32
#if QT_VERSION < 0x040000
	if ( QPaintDevice::x11AppDisplay() )
#else
	if ( QX11Info::display() )
#endif
#endif
		QApplication::clipboard()->setText( mView->myBuffer()->getText( interval ).join( "\n" ), QClipboard::Selection );

}

