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
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

#ifndef YZ_MODE_INSERT_H
#define YZ_MODE_INSERT_H

#include "mode.h"
#include "mode_command.h"
#include "yzismacros.h"

class YMode;
class YView;

/**
 * Insert mode
 *
 * The mode that text is inserted in.
 */
class YZIS_EXPORT YModeInsert : public YModeCommand
{
public:
    YModeInsert();
    virtual ~YModeInsert()
    {}

    virtual void initMotionPool();
    virtual void initCommandPool();

    virtual void enter( YView *view );
    virtual void leave( YView *view );
    virtual CmdState execCommand( YView *view, const YKeySequence &inputs,
				  YKeySequence::const_iterator &parsePos);

    virtual CmdState commandInsert(const YCommandArgs &args);
    CmdState insertFromBelow( const YCommandArgs &args );
    CmdState insertFromAbove( const YCommandArgs &args );
    CmdState completion( const YCommandArgs &args );
    CmdState completionNext( const YCommandArgs &args );
    CmdState completionPrevious( const YCommandArgs &args );
    virtual CmdState backspace( const YCommandArgs &args );
    CmdState deleteWordBefore( const YCommandArgs &args );
    CmdState deleteLineBefore( const YCommandArgs &args );
    CmdState commandEnter( const YCommandArgs &args );

    virtual CmdState addText( YView* mView, const QString& text );

    // Dubious
    CmdState deleteChar( const YCommandArgs &args );
    CmdState deleteCharBackwards( const YCommandArgs &args );
    void imBegin( YView* );
    void imCompose( YView* mView, const QString& entry );
    void imEnd( YView* mView, const QString& entry );

    void initModifierKeys();
protected :
    QString m_imPreedit;
};

/**
 * Replace mode
 */
class YZIS_EXPORT YModeReplace : public YModeInsert
{
public :
    YModeReplace();
    virtual ~YModeReplace()
    {}

    virtual CmdState addText( YView* mView, const QString& key );
    virtual CmdState commandInsert( const YCommandArgs &args );
    virtual CmdState backspace( const YCommandArgs &args );
};

#endif

