/*  This file is part of the Yzis libraries
*  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
*  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
*  Copyright (C) 2007 Philippe Fremy <phil at freehackers dot org>
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

#include "mode_pool.h"
#include "session.h"
#include "debug.h"
#include "mapping.h"
#include "view.h"

using namespace yzis;

// ====================================================================
//
//                          YModePool
//
// ====================================================================

#define dbg() yzDebug("YModePool")
#define err() yzError("YModePool")

YModePool::YModePool( YView* view )
{
    mView = view;
    mModes = YSession::self()->getModes();
    mapMode = 0;
    mRegisterKeys = false;
    mStop = false;
    stack.push_front(mModes[YMode::ModeCommand]);
    stack.front()->enter(mView);
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
CmdState YModePool::sendKey( const YKey &key )
{
    CmdState state = CmdOk;

    mKey = key;

    //check mappings
    mapMode |= current()->mapMode();
    bool map = false;
    QString mapped = mView->getInputBuffer().toString() + mKey.toString();
    dbg() << "Looking mappings for " << mapped << endl;
    bool pendingMapp = YZMapping::self()->applyMappings( mapped, mapMode, &map );
    if (pendingMapp)
      dbg() << "Pending mapping on " << mapped << endl;
    if ( map ) {
        dbg() << "input buffer was remapped to: " << mapped << endl;
        mView->purgeInputBuffer();
        mapMode = 0;
        YKeySequence inputs(mapped);
        YKeySequence::const_iterator parsePos = inputs.begin();
            return YSession::self()->sendMultipleKeys( mView, inputs, parsePos );
    }

    dbg() << "Appending to input buffer " << mKey.toString() << endl;
    mView->appendInputBuffer( mKey );
    
    YKeySequence::const_iterator parsePos = mView->getInputBuffer().begin();
    state = stack.front()->execCommand( mView, mView->getInputBuffer(), parsePos);
    
    if ( mStop ) return CmdStopped;
    switch (state) {
    case CmdError:
        dbg() << "CmdState = CmdError" << endl;
        if (pendingMapp) break;
    case CmdStopped:
    case CmdOk:
        mView->purgeInputBuffer();
        // stopped is false if state == CmdOk
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
    return state;
}
CmdState YModePool::replayKey()
{
    return YSession::self()->sendKey( mView, mKey );
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
    dbg() << "push( " << mode << " )" << endl;
    // unregisterModifierKeys();
    stack.push_front( mModes[ mode ] );
    if (mRegisterKeys) registerModifierKeys();
    dbg() << "push(): entering mode " << stack.front()->toString() << endl;
    stack.front()->enter( mView );
    mView->updateMode();
    dbg() << "push() done" << endl;
}
void YModePool::pop( bool leave_me )
{
    dbg() << "pop( leave_me=" << leave_me << " )" << endl;
    if ( mStop ) return ;
    mView->commitUndoItem();
    mView->purgeInputBuffer();
    // unregisterModifierKeys();
    if ( ! stack.isEmpty() ) {
        if ( leave_me ) {
            dbg() << "pop(): leaving mode " << stack.front()->toString() << endl;
            stack.front()->leave( mView );
        }
        stack.pop_front();
    }
    if ( stack.isEmpty() )
        push( YMode::ModeCommand );
    else
        mView->updateMode();
    if (mRegisterKeys) registerModifierKeys();
}

void YModePool::pop( ModeType mode )
{
    dbg() << "pop( " << mode << " )" << endl;
    if ( mStop ) { 
        dbg() << "pop(): stop!" << endl;
        return ;
    }
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
        mView->updateMode();
    if (mRegisterKeys) registerModifierKeys();
    dbg() << "pop() done" << endl;
}



