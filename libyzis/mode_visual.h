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


    void commandInsert( const YCommandArgs& args );
    void commandAppend( const YCommandArgs& args );
    void gotoExMode( const YCommandArgs& args );
    void movetoExMode( const YCommandArgs& args );
    void movetoInsertMode( const YCommandArgs& args );
    void escape( const YCommandArgs& args );
    void changeWholeLines(const YCommandArgs &args);
    void deleteWholeLines(const YCommandArgs &args);
    void yankWholeLines(const YCommandArgs &args);
    void yank(const YCommandArgs &args);
    void toUpperCase( const YCommandArgs& args );
    void toLowerCase( const YCommandArgs& args );
    void translateToVisual( const YCommandArgs& args );
    void translateToVisualLine( const YCommandArgs& args );
    void translateToVisualBlock( const YCommandArgs& args );

    virtual YInterval interval(const YCommandArgs &args);

protected:
    virtual YInterval buildBufferInterval( YView* mView, const YViewCursor& from, const YViewCursor& to );
    virtual YInterval buildScreenInterval( YView* mView, const YViewCursor& from, const YViewCursor& to );
    bool mEntireLines;
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

