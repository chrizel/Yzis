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
#include "mode_pool.h"
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
    case CmdStopped: out << "CmdStopped"; break;
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
    mIsEditMode = false;
    mIsSelMode = false;
    mIsCmdLineMode = false;
    mIM = false;
    mMapMode = MapNormal;
    mRegistered = false;
}
ModeType YMode::modeType() const
{
    return mType;
}
const QString& YMode::toString() const
{
    return mString;
}
bool YMode::isEditMode() const
{
    return mIsEditMode;
}
bool YMode::isSelMode() const
{
    return mIsSelMode;
}
bool YMode::isCommandLineMode() const
{
    return mIsCmdLineMode;
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
    mIsEditMode = true;
    mIsCmdLineMode = false;
    mIsSelMode = false;
}
void YModeIntro::enter( YView* mView )
{
	YRawData content;
	content << mView->centerLine(VERSION_CHAR_LONG);
#if defined VERSION_CHAR_DATE
	content << mView->centerLine(VERSION_CHAR_DATE);
#endif
    content << mView->centerLine( VERSION_CHAR_ST );
    content << "";
    content << mView->centerLine( "http://www.yzis.org" );
    content << mView->centerLine( "contact/patches/requests: yzis-dev@yzis.org" );
    content << "";
    content << mView->centerLine( "Yzis is distributed under the terms of the GPL v2" );
    content << "";
    content << mView->centerLine( "please report bugs at http://bugs.yzis.org" );

	int spaceh = mView->getLinesVisible() - content.count();
	int i = 0;
	for ( ; i < spaceh / 2; ++i ) {
		content.insert(0, "");
	}
	for ( ; i < spaceh; ++i ) {
		content << "";
	}

    /* Don't record these in the undo list */
    mView->buffer()->undoBuffer()->setInsideUndo(true);

    mView->gotoLinePosition(0, 0);
	mView->buffer()->clearText();
	mView->buffer()->insertRegion(YCursor(0,0), content);
	mView->buffer()->setChanged(false);

    mView->buffer()->undoBuffer()->setInsideUndo(false);
}
void YModeIntro::leave( YView* mView )
{
    YBuffer* mBuffer = mView->buffer();
    mBuffer->undoBuffer()->setInsideUndo( true );
    mView->gotoLinePosition(0 , 0);
    mBuffer->clearText();
    mBuffer->undoBuffer()->setInsideUndo( false );
    mBuffer->setChanged( false );
    mView->recalcScreen();
}
CmdState YModeIntro::execCommand( YView* mView, const YKeySequence &keys, YKeySequence::const_iterator &parsePos )
{
    Q_UNUSED(keys);
    Q_UNUSED(parsePos);
    mView->modePool()->change( ModeCommand );
    return mView->modePool()->replayKey();
}

