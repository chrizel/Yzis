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

#ifndef YZ_MODE_VISUAL_H
#define YZ_MODE_VISUAL_H

#include "mode_command.h"
#include "yzismacros.h"

class YMode;
class YView;
class YViewCursor;

/**
  * @short Visual mode (selecting with v, shift+v or control-v)
  */
class YZIS_EXPORT YModeVisual : public YModeCommand
{
public:
    YModeVisual();
    virtual ~YModeVisual();

    virtual void initCommandPool();
    virtual void initVisualCommandPool();

    virtual void enter( YView* mView );
    virtual void leave( YView* mView );

    virtual void cursorMoved( YView* mView );
    virtual void toClipboard( YView* mView );


    CmdState commandInsert( const YCommandArgs& args );
    CmdState commandAppend( const YCommandArgs& args );
    CmdState gotoExMode( const YCommandArgs& args );
    CmdState movetoExMode( const YCommandArgs& args );
    CmdState movetoInsertMode( const YCommandArgs& args );
    CmdState escape( const YCommandArgs& args );
    CmdState changeWholeLines(const YCommandArgs &args);
    CmdState deleteWholeLines(const YCommandArgs &args);
    CmdState yankWholeLines(const YCommandArgs &args);
    CmdState yank(const YCommandArgs &args);
    CmdState toUpperCase( const YCommandArgs& args );
    CmdState toLowerCase( const YCommandArgs& args );
    CmdState translateToVisual( const YCommandArgs& args );
    CmdState translateToVisualLine( const YCommandArgs& args );
    CmdState translateToVisualBlock( const YCommandArgs& args );

    virtual YInterval interval(const YCommandArgs &args, CmdState *state);

protected:
    virtual YInterval buildBufferInterval( YView* mView, const YViewCursor& from, const YViewCursor& to );
    virtual YInterval buildScreenInterval( YView* mView, const YViewCursor& from, const YViewCursor& to );
    bool mEntireLines;
	QMap<YView*, YViewCursor> startViewCursor;
};

/**
 * Visual line mode
 */
class YModeVisualLine : public YModeVisual
{
public:
    YModeVisualLine();
    virtual ~YModeVisualLine();

protected:
    virtual YInterval buildBufferInterval( YView* mView, const YViewCursor& from, const YViewCursor& to );
    virtual YInterval buildScreenInterval( YView* mView, const YViewCursor& from, const YViewCursor& to );
};

/**
 * Visual block mode
 */
class YModeVisualBlock : public YModeVisual
{
public:
    YModeVisualBlock();
    virtual ~YModeVisualBlock();

    virtual void cursorMoved( YView* mView );
};

#endif

