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

// ====================================================================
//
//                          YZMode
//
// ====================================================================

#define dbg() yzDebug("YZMode")
#define err() yzError("YZMode")

YZDebugStream& operator<<( YZDebugStream& out, const CmdState & state )
{
    switch( state ) {
        case CmdError: out << "CmdError"; break;
        case CmdNotYetValid: out << "CmdNotYetValid"; break;
        case CmdOperatorPending: out << "CmdOperatorPending"; break;
        case CmdOk: out << "CmdOk"; break;
        case CmdQuit: out << "CmdQuit"; break;
    }
    return out;
}

YZDebugStream& operator<<( YZDebugStream& out, const YZMode::ModeType & type )
{
    switch( type ) {
        case YZMode::ModeCommand: out << "ModeCommand"; break; 
		case YZMode::ModeInsert: out << "ModeInsert"; break;
		case YZMode::ModeReplace: out << "ModeReplace"; break;
		case YZMode::ModeEx: out << "ModeEx"; break;
		case YZMode::ModeSearch: out << "ModeSearch"; break;
		case YZMode::ModeSearchBackward: out << "ModeSearchBackward"; break;
		case YZMode::ModeIntro: out << "ModeIntro"; break;
		case YZMode::ModeCompletion: out << "ModeCompletion"; break;
		case YZMode::ModeVisual: out << "ModeVisual"; break;
		case YZMode::ModeVisualLine: out << "ModeVisualLine"; break;
		case YZMode::ModeVisualBlock: out << "ModeVisualBlock"; break;
    }
    return out;
}

YZMode::YZMode() {
	mString = "if you see me, there is a problem :)";
	mEditMode = false;
	mSelMode = false;
	mIM = false;
	mMapMode = MapNormal;
	mRegistered = false;
}
ModeType YZMode::type() const {
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
MapMode YZMode::mapMode() const {
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


// ====================================================================
//
//                          YZModeIntro
//
// ====================================================================

#undef dbg
#undef err
#define dbg() yzDebug("YZModeIntro")
#define err() yzError("YZModeIntro")

YZModeIntro::YZModeIntro() : YZMode() {
	mType = ModeIntro;
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
CmdState YZModeIntro::execCommand( YZView* mView, const QString& ) {
	mView->modePool()->change( ModeCommand );
	mView->modePool()->replayKey();
	return CmdOk;
}

// ====================================================================
//
//                          YZModePool
//
// ====================================================================

#undef dbg
#undef err
#define dbg() yzDebug("YZModePool")
#define err() yzError("YZModePool")

YZModePool::YZModePool( YZView* view ) {
	mView = view;
	mModes = YZSession::self()->getModes();
	mapMode = 0;
	mRegisterKeys = false;
	mStop = false;
}
YZModePool::~YZModePool() {
    // dbg() << HERE() << endl;
	stop();
}
void YZModePool::stop() {
    // dbg() << HERE() << endl;
	mStop = true;
//	dbg() << "YZModePool stopped for view " << mView->myId << endl;
}
void YZModePool::sendKey( const QString& key, const QString& modifiers ) {
	mKey = key;
	mModifiers = modifiers;
	
	//check mappings
	mapMode |= current()->mapMode();
	bool map = false;
	mView->saveInputBuffer();
	QString mapped = mView->getInputBuffer(); // + modifiers + key;
//	dbg() << "Looking mappings for " << mapped << endl;
	bool pendingMapp = YZMapping::self()->applyMappings( mapped, mapMode, &map );
//	if (pendingMapp)
//		dbg() << "Pending mapping on " << mapped << endl;
	if ( map ) {
//		dbg() << "input buffer was remapped to: " << mapped << endl;
		mView->purgeInputBuffer();
		mapMode = 0;
		YZSession::self()->sendMultipleKeys( mView, mapped );
		return;
	}
	CmdState state = stack.front()->execCommand( mView, mView->getInputBuffer() );
	if ( mStop ) return;
	switch(state) {
		case CmdError: 
			dbg() << "CmdState = CmdError" << endl;
			if (pendingMapp) break;
		case CmdOk:
			mView->purgeInputBuffer();
			mapMode = 0;
			break;
		case CmdOperatorPending:
			dbg() << "CmdState = CmdOperatorPending" << endl;
			mapMode = MapPendingOp;
			break;
		case CmdQuit:
			dbg() << "CmdState = CmdQuit" << endl;
		default:
			break;
	}
}
void YZModePool::replayKey() {
	YZSession::self()->sendKey( mView, mKey, mModifiers );
}
YZMode* YZModePool::current() const {
	return stack.front();
}
ModeType YZModePool::currentType() const {
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
	dbg() << "register keys " << keys << endl;
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
	dbg() << "unregister keys " << keys << endl;
	for( unsigned i = 0; i < size; i++ )
		mView->unregisterModifierKeys( keys.at(i) );
	stack.front()->setRegistered( false );
}

void YZModePool::change( ModeType mode, bool leave_me ) {
	pop( leave_me );
	push( mode );
}
void YZModePool::push( ModeType mode ) {
//	unregisterModifierKeys();
	stack.push_front( mModes[ mode ] );
	if (mRegisterKeys) registerModifierKeys();
	dbg() << "entering mode " << stack.front()->toString() << endl;
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
			dbg() << "leaving mode " << stack.front()->toString() << endl;
			stack.front()->leave( mView );
		}
		stack.pop_front();
	}
	if ( stack.isEmpty() )
		push( YZMode::ModeCommand );
	else
		mView->modeChanged();
	if (mRegisterKeys) registerModifierKeys();
}
void YZModePool::pop( ModeType mode ) {
	if ( mStop ) return;
//	unregisterModifierKeys();
	mView->commitUndoItem();
	mView->purgeInputBuffer();
	// do not leave two times the same mode
	QList<YZMode*> leaved;
	while ( stack.size() > 0 && stack.front()->type() != mode ) {
		if ( ! leaved.contains( stack.front() ) ) {
			dbg() << "leaving mode " << stack.front()->toString() << endl;
			stack.front()->leave( mView );
			leaved.append( stack.front() );
		}
		stack.pop_front();
	}
	if ( stack.isEmpty() )
		push( YZMode::ModeCommand );
	else
		mView->modeChanged();
	if (mRegisterKeys) registerModifierKeys();
}


