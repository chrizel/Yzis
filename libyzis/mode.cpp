/*  This file is part of the Yzis libraries
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

#include "mode.h"
#include "portability.h"
#include "debug.h"

#include "buffer.h"
#include "mapping.h"
#include "session.h"
#include "undo.h"
#include "view.h"

using namespace yzis;

YZMode::YZMode() {
	mString = "if you see me, there is a problem :)";
	mEditMode = false;
	mSelMode = false;
	mIM = false;
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
bool YZMode::isSelMode() const {
	return mSelMode;
}
bool YZMode::supportsInputMethod() const {
	return mIM;
}
mapping_t YZMode::mapMode() const {
	return mMapMode;
}
QStringList YZMode::modifierKeys() const {
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
void YZMode::imBegin( YZView* ) {
}
void YZMode::imCompose( YZView*, const QString& ) {
}
void YZMode::imEnd( YZView*, const QString& ) {
}


/**
 * YZModeIntro
 */

YZModeIntro::YZModeIntro() : YZMode() {
	mType = MODE_INTRO;
	mString = _("[ Introduction ]");
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
	mView->saveInputBuffer();
	QString mapped = mView->getInputBuffer(); // + modifiers + key;
//	yzDebug() << "Looking mappings for " << mapped << endl;
	bool pendingMapp = YZMapping::self()->applyMappings( mapped, mapMode, &map );
//	if (pendingMapp)
//		yzDebug() << "Pending mapping on " << mapped << endl;
	if ( map ) {
//		yzDebug() << "input buffer was remapped to : " << mapped << endl;
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
			mView->purgeInputBuffer();
			mapMode = 0;
			break;
		case OPERATOR_PENDING:
			yzDebug() << "cmd_state = OPERATOR_PENDING" << endl;
			mapMode = pendingop;
			break;
		case CMD_QUIT:
			yzDebug() << "cmd_state = CMD_QUIT" << endl;
		default:
			break;
	}
}
void YZModePool::replayKey() {
	mView->sendKey( mKey, mModifiers );
}
YZMode* YZModePool::current() const {
	return stack.front();
}
modeType YZModePool::currentType() const {
	return current()->type();
}
void YZModePool::registerModifierKeys() {
	if ( mStop ) return;

	QStringList mModifierKeys;
	YZModeMap::Iterator it;
	for( it = mModes.begin(); it != mModes.end(); ++it ) {
		mModifierKeys += it.value()->modifierKeys();
	}
	mModifierKeys.sort();
	unsigned int size = mModifierKeys.size();
	QString last, current;
	for ( unsigned int i = 0; i < size; i++ ) {
		current = mModifierKeys.at(i);
		if ( current != last ) {
			mView->registerModifierKeys( current );
			last = current;
		}
	}

#if 0
	
	
	mRegisterKeys = true;
	if ( stack.isEmpty() || stack.front()->registered() ) return;
	QStringList keys = stack.front()->modifierKeys();
	unsigned int size = keys.size();
	yzDebug() << "register keys " << keys << endl;
	for( unsigned i = 0; i < size; i++ )
		mView->registerModifierKeys( keys.at(i) );
	stack.front()->setRegistered( true );
#endif
}
void YZModePool::unregisterModifierKeys() {
	if ( mStop ) return;
	if ( stack.isEmpty() || !stack.front()->registered() ) return;
	QStringList keys = stack.front()->modifierKeys();
	unsigned int size = keys.size();
	yzDebug() << "unregister keys " << keys << endl;
	for( unsigned i = 0; i < size; i++ )
		mView->unregisterModifierKeys( keys.at(i) );
	stack.front()->setRegistered( false );
}

void YZModePool::change( modeType mode, bool leave_me ) {
	pop( leave_me );
	push( mode );
}
void YZModePool::push( modeType mode ) {
//	unregisterModifierKeys();
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
//	unregisterModifierKeys();
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
//	unregisterModifierKeys();
	mView->commitUndoItem();
	mView->purgeInputBuffer();
	// do not leave two times the same mode
	QList<YZMode*> leaved;
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


