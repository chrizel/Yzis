/*  This file is part of the Yzis libraries
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
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

#include "mode.h"

#include "debug.h"

#include "buffer.h"
#include "mapping.h"
#include "session.h"
#include "undo.h"

YZMode::YZMode() {
	mString = "if you see me, there is a problem :)";
	mEditMode = false;
	mMapMode = normal;
	mRegistered = false;
}
modeType YZMode::type() const {
	return mType;
}
const QString& YZMode::toString() const {
	return mString;
}
bool YZMode::isEditMode() const {
	return mEditMode;
}
mapping_t YZMode::mapMode() const {
	return mMapMode;
}
QStringList YZMode::modifierKeys() {
	return mModifierKeys;
}
bool YZMode::registered() const {
	return mRegistered;
}
void YZMode::setRegistered( bool registered ) {
	mRegistered = registered;
}
void YZMode::init() {
	initModifierKeys();
}
void YZMode::initModifierKeys() {
}
void YZMode::enter( YZView* ) {
}
void YZMode::leave( YZView* ) {
}
void YZMode::cursorMoved( YZView* ) {
}


/**
 * YZModeIntro
 */

YZModeIntro::YZModeIntro() : YZMode() {
	mType = MODE_INTRO;
	mString = QObject::tr("[ Introduction ]");
}
void YZModeIntro::enter( YZView* mView ) {
	YZBuffer* mBuffer = mView->myBuffer();
	unsigned int i;
	unsigned int mLinesVis = mView->getLinesVisible();
	unsigned int linesInIntro = 11; // Update this is if you change # of lines in message
	unsigned int vMargin = mLinesVis > linesInIntro ? mLinesVis - linesInIntro : 0;
	vMargin = ( vMargin + 1 ) / 2; // round up to have enough lines so '~' isn't shown

	/* Don't record these in the undo list */
	mBuffer->undoBuffer()->setInsideUndo( true );

	mView->gotoxy( 0, 0 );
	for (i = 0; i < vMargin; i++ ) mBuffer->appendLine("");
	mBuffer->appendLine( mView->centerLine( VERSION_CHAR_LONG ) );
	if ( VERSION_CHAR_ST == VERSION_CHAR_STATE2 )
		mBuffer->appendLine( mView->centerLine( VERSION_CHAR_DATE ) );
	mBuffer->appendLine( mView->centerLine( VERSION_CHAR_ST  ) );
	mBuffer->appendLine( "" );
	mBuffer->appendLine( mView->centerLine( "http://www.yzis.org" ) );
	mBuffer->appendLine( mView->centerLine( "contact/patches/requests: yzis-dev@yzis.org" ) );
	mBuffer->appendLine( "" );
	mBuffer->appendLine( mView->centerLine( "Yzis is distributed under the terms of the GPL v2" ) );
	mBuffer->appendLine( "" );
	mBuffer->appendLine( mView->centerLine( "please report bugs at http://bugs.yzis.org" ) );
	for ( i = 0; i < vMargin; i++ ) mBuffer->appendLine( "" );
	mBuffer->setChanged( false );

	mBuffer->undoBuffer()->setInsideUndo( false );
	mView->refreshScreen();
}
void YZModeIntro::leave( YZView* mView ) {
	YZBuffer* mBuffer = mView->myBuffer();
	mBuffer->undoBuffer()->setInsideUndo( true );
	mView->gotoxy( 0, 0 );
	mBuffer->clearText();
	mBuffer->undoBuffer()->setInsideUndo( false );
	mBuffer->setChanged( false );
	mView->recalcScreen();
}
cmd_state YZModeIntro::execCommand( YZView* mView, const QString& ) {
	mView->modePool()->change( MODE_COMMAND );
	mView->modePool()->replayKey();
	return CMD_OK;
}


/**
 * YZModePool
 */

YZModePool::YZModePool( YZView* view ) {
	mView = view;
	mModes = YZSession::me->getModes();
	mapMode = 0;
	mRegisterKeys = false;
	mStop = false;
}
YZModePool::~YZModePool() {
}
void YZModePool::stop() {
	mStop = true;
//	yzDebug() << "YZModePool stopped for view " << mView->myId << endl;
}
void YZModePool::sendKey( const QString& key, const QString& modifiers ) {
	mKey = key;
	mModifiers = modifiers;
	
	//check mappings
	mapMode |= current()->mapMode();
	bool map = false;
	QString mapped = modifiers + key;
	bool pendingMapp = YZMapping::self()->applyMappings( mapped, mapMode, &map );
	if ( map ) {
		mView->purgeInputBuffer();
		mapMode = 0;
		mView->sendMultipleKey( mapped );
		return;
	}
	cmd_state state = stack.front()->execCommand( mView, mView->getInputBuffer() );
	if ( mStop ) return;
	switch(state) {
		case CMD_ERROR: 
			yzDebug() << "cmd_state = CMD_ERROR" << endl;
			if (pendingMapp) break;
		case CMD_OK:
			yzDebug() << "cmd_state = CMD_OK" << endl;
			mView->purgeInputBuffer();
			mapMode = 0;
			break;
		case OPERATOR_PENDING:
			yzDebug() << "cmd_state = OPERATOR_PENDING" << endl;
			mapMode = pendingop;
			break;
		case CMD_QUIT:
		default:
			break;
	}
}
void YZModePool::replayKey() {
	mView->sendKey( mKey, mModifiers );
}
YZMode* YZModePool::current() {
	return stack.front();
}
modeType YZModePool::currentType() {
	return current()->type();
}
void YZModePool::registerModifierKeys() {
	if ( mStop ) return;
	mRegisterKeys = true;
	if ( stack.isEmpty() || stack.front()->registered() ) return;
	QStringList keys = stack.front()->modifierKeys();
	unsigned int size = keys.size();
	yzDebug() << "register keys " << keys << endl;
	for( unsigned i = 0; i < size; i++ )
#if QT_VERSION < 0x040000
		mView->registerModifierKeys( (*keys.at(i)) );
#else
		mView->registerModifierKeys( keys.at(i) );
#endif
	stack.front()->setRegistered( true );
}
void YZModePool::unregisterModifierKeys() {
	if ( mStop ) return;
	if ( stack.isEmpty() || !stack.front()->registered() ) return;
	QStringList keys = stack.front()->modifierKeys();
	unsigned int size = keys.size();
	yzDebug() << "unregister keys " << keys << endl;
	for( unsigned i = 0; i < size; i++ )
#if QT_VERSION < 0x040000
		mView->unregisterModifierKeys( (*keys.at(i)) );
#else
		mView->unregisterModifierKeys( keys.at(i) );
#endif
	stack.front()->setRegistered( false );
}

void YZModePool::change( modeType mode, bool leave_me ) {
	pop( leave_me );
	push( mode );
}
void YZModePool::push( modeType mode ) {
	unregisterModifierKeys();
	stack.push_front( mModes[ mode ] );
	if (mRegisterKeys) registerModifierKeys();
	yzDebug() << "entering mode " << stack.front()->toString() << endl;
	stack.front()->enter( mView );
	mView->modeChanged();
}
void YZModePool::pop( bool leave_me ) {
	if ( mStop ) return;
	mView->commitUndoItem();
	mView->purgeInputBuffer();
	unregisterModifierKeys();
	if ( ! stack.isEmpty() ) {
		if ( leave_me ) {
			yzDebug() << "leaving mode " << stack.front()->toString() << endl;
			stack.front()->leave( mView );
		}
		stack.pop_front();
	}
	if ( stack.isEmpty() )
		push( YZMode::MODE_COMMAND );
	else
		mView->modeChanged();
	if (mRegisterKeys) registerModifierKeys();
}
void YZModePool::pop( modeType mode ) {
	if ( mStop ) return;
	unregisterModifierKeys();
	mView->commitUndoItem();
	mView->purgeInputBuffer();
	// do not leave two times the same mode
#if QT_VERSION < 0x040000
	QValueList<YZMode*> leaved;
#else
	QList<YZMode*> leaved;
#endif
	while ( stack.size() > 0 && stack.front()->type() != mode ) {
		if ( ! leaved.contains( stack.front() ) ) {
			yzDebug() << "leaving mode " << stack.front()->toString() << endl;
			stack.front()->leave( mView );
			leaved.append( stack.front() );
		}
		stack.pop_front();
	}
	if ( stack.isEmpty() )
		push( YZMode::MODE_COMMAND );
	else
		mView->modeChanged();
	if (mRegisterKeys) registerModifierKeys();
}


