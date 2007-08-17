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
//                          YMode
//
// ====================================================================

#define dbg() yzDebug("YMode")
#define err() yzError("YMode")

YDebugStream& operator<<( YDebugStream& out, const CmdState & state )
{
    switch ( state ) {
    case CmdError: out << "CmdError"; break;
    case CmdNotYetValid: out << "CmdNotYetValid"; break;
    case CmdOperatorPending: out << "CmdOperatorPending"; break;
    case CmdOk: out << "CmdOk"; break;
    case CmdQuit: out << "CmdQuit"; break;
    }
    return out;
}

YDebugStream& operator<<( YDebugStream& out, const YMode::ModeType & type )
{
    switch ( type ) {
    case YMode::ModeCommand: out << "ModeCommand"; break;
    case YMode::ModeInsert: out << "ModeInsert"; break;
    case YMode::ModeReplace: out << "ModeReplace"; break;
    case YMode::ModeEx: out << "ModeEx"; break;
    case YMode::ModeSearch: out << "ModeSearch"; break;
    case YMode::ModeSearchBackward: out << "ModeSearchBackward"; break;
    case YMode::ModeIntro: out << "ModeIntro"; break;
    case YMode::ModeCompletion: out << "ModeCompletion"; break;
    case YMode::ModeVisual: out << "ModeVisual"; break;
    case YMode::ModeVisualLine: out << "ModeVisualLine"; break;
    case YMode::ModeVisualBlock: out << "ModeVisualBlock"; break;
    }
    return out;
}

YMode::YMode()
{
    mString = "if you see me, there is a problem :)";
    mEditMode = false;
    mSelMode = false;
    mIM = false;
    mMapMode = MapNormal;
    mRegistered = false;
}
ModeType YMode::type() const
{
    return mType;
}
const QString& YMode::toString() const
{
    return mString;
}
bool YMode::isEditMode() const
{
    return mEditMode;
}
bool YMode::isSelMode() const
{
    return mSelMode;
}
bool YMode::supportsInputMethod() const
{
    return mIM;
}
MapMode YMode::mapMode() const
{
    return mMapMode;
}
QStringList YMode::modifierKeys() const
{
    return mModifierKeys;
}
bool YMode::registered() const
{
    return mRegistered;
}
void YMode::setRegistered( bool registered )
{
    mRegistered = registered;
}
void YMode::init()
{
    initModifierKeys();
}
void YMode::initModifierKeys()
{}
void YMode::enter( YView* )
{}
void YMode::leave( YView* )
{}
void YMode::cursorMoved( YView* )
{}
void YMode::imBegin( YView* )
{}
void YMode::imCompose( YView*, const QString& )
{}
void YMode::imEnd( YView*, const QString& )
{}


// ====================================================================
//
//                          YModeIntro
//
// ====================================================================

#undef dbg
#undef err
#define dbg() yzDebug("YModeIntro")
#define err() yzError("YModeIntro")

YModeIntro::YModeIntro() : YMode()
{
    mType = ModeIntro;
    mString = _("[ Introduction ]");
}
void YModeIntro::enter( YView* mView )
{
    YBuffer* mBuffer = mView->myBuffer();
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
    mBuffer->appendLine( mView->centerLine( VERSION_CHAR_ST ) );
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
void YModeIntro::leave( YView* mView )
{
    YBuffer* mBuffer = mView->myBuffer();
    mBuffer->undoBuffer()->setInsideUndo( true );
    mView->gotoxy( 0, 0 );
    mBuffer->clearText();
    mBuffer->undoBuffer()->setInsideUndo( false );
    mBuffer->setChanged( false );
    mView->recalcScreen();
}
CmdState YModeIntro::execCommand( YView* mView, const QString& )
{
    mView->modePool()->change( ModeCommand );
    mView->modePool()->replayKey();
    return CmdOk;
}

// ====================================================================
//
//                          YModePool
//
// ====================================================================

#undef dbg
#undef err
#define dbg() yzDebug("YModePool")
#define err() yzError("YModePool")

YModePool::YModePool( YView* view )
{
    mView = view;
    mModes = YSession::self()->getModes();
    mapMode = 0;
    mRegisterKeys = false;
    mStop = false;
}
YModePool::~YModePool()
{
    // dbg() << HERE() << endl;
    stop();
}
void YModePool::stop()
{
    // dbg() << HERE() << endl;
    mStop = true;
    // dbg() << "YModePool stopped for view " << mView->myId << endl;
}
void YModePool::sendKey( const QString& key, const QString& modifiers )
{
    mKey = key;
    mModifiers = modifiers;

    //check mappings
    mapMode |= current()->mapMode();
    bool map = false;
    mView->saveInputBuffer();
    QString mapped = mView->getInputBuffer() + mModifiers + mKey;
    dbg() << "Looking mappings for " << mapped << endl;
    bool pendingMapp = YZMapping::self()->applyMappings( mapped, mapMode, &map );
    if (pendingMapp)
      dbg() << "Pending mapping on " << mapped << endl;
    if ( map ) {
        dbg() << "input buffer was remapped to: " << mapped << endl;
        mView->purgeInputBuffer();
        mapMode = 0;
        YSession::self()->sendMultipleKeys( mView, mapped );
        return ;
    }

	// shift might have been used for remapping, 
	// so we remove it only after having check the mappings
	// we remove it so that one can type upper case letters without entering a <SHIFT> on screen
    if ( mModifiers.contains ("<SHIFT>")) {
        mKey = mKey.toUpper();
        mModifiers.remove( "<SHIFT>" );
    }
	dbg() << "Appending to input buffer " << mModifiers + mKey << endl;
    mView->appendInputBuffer( mModifiers + mKey );

    CmdState state = stack.front()->execCommand( mView, mView->getInputBuffer() );
    if ( mStop ) return ;
    switch (state) {
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
void YModePool::replayKey()
{
    YSession::self()->sendKey( mView, mKey, mModifiers );
}
YMode* YModePool::current() const
{
    return stack.front();
}
ModeType YModePool::currentType() const
{
    return current()->type();
}
void YModePool::registerModifierKeys()
{
    if ( mStop ) return ;

    QStringList mModifierKeys;
    YModeMap::Iterator it;
    for ( it = mModes.begin(); it != mModes.end(); ++it ) {
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
    if ( stack.isEmpty() || stack.front()->registered() ) return ;
    QStringList keys = stack.front()->modifierKeys();
    unsigned int size = keys.size();
    dbg() << "register keys " << keys << endl;
    for ( unsigned i = 0; i < size; i++ )
        mView->registerModifierKeys( keys.at(i) );
    stack.front()->setRegistered( true );
#endif
}
void YModePool::unregisterModifierKeys()
{
    if ( mStop ) return ;
    if ( stack.isEmpty() || !stack.front()->registered() ) return ;
    QStringList keys = stack.front()->modifierKeys();
    unsigned int size = keys.size();
    dbg() << "unregister keys " << keys << endl;
    for ( unsigned i = 0; i < size; i++ )
        mView->unregisterModifierKeys( keys.at(i) );
    stack.front()->setRegistered( false );
}

void YModePool::change( ModeType mode, bool leave_me )
{
    pop( leave_me );
    push( mode );
}
void YModePool::push( ModeType mode )
{
    // unregisterModifierKeys();
    stack.push_front( mModes[ mode ] );
    if (mRegisterKeys) registerModifierKeys();
    dbg() << "entering mode " << stack.front()->toString() << endl;
    stack.front()->enter( mView );
    mView->guiModeChanged();
}
void YModePool::pop( bool leave_me )
{
    if ( mStop ) return ;
    mView->commitUndoItem();
    mView->purgeInputBuffer();
    // unregisterModifierKeys();
    if ( ! stack.isEmpty() ) {
        if ( leave_me ) {
            dbg() << "leaving mode " << stack.front()->toString() << endl;
            stack.front()->leave( mView );
        }
        stack.pop_front();
    }
    if ( stack.isEmpty() )
        push( YMode::ModeCommand );
    else
        mView->guiModeChanged();
    if (mRegisterKeys) registerModifierKeys();
}
void YModePool::pop( ModeType mode )
{
    if ( mStop ) return ;
    // unregisterModifierKeys();
    mView->commitUndoItem();
    mView->purgeInputBuffer();
    // do not leave two times the same mode
    QList<YMode*> leaved;
    while ( stack.size() > 0 && stack.front()->type() != mode ) {
        if ( ! leaved.contains( stack.front() ) ) {
            dbg() << "leaving mode " << stack.front()->toString() << endl;
            stack.front()->leave( mView );
            leaved.append( stack.front() );
        }
        stack.pop_front();
    }
    if ( stack.isEmpty() )
        push( YMode::ModeCommand );
    else
        mView->guiModeChanged();
    if (mRegisterKeys) registerModifierKeys();
}


