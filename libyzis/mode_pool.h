/*  This file is part of the Yzis libraries
*  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
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

#ifndef YZ_MODEPOOL_H
#define YZ_MODEPOOL_H

/* Yzis */
#include "mode.h"

/** @file modepool.h
  * Some documentation
  *
  */

/**
 * Keeps track of if the different key mode in yzis.
 */
class YZIS_EXPORT YModePool
{
public:
    YModePool( YView* view );
    virtual ~YModePool();

    bool sendKey( const QString& key, const QString& modifiers );
    void replayKey();

    /**
     * pop current @arg mode and push @arg mode
     */
    void change( ModeType mode, bool leave_me = true );

    /**
     * push @arg mode
     */
    void push( ModeType mode );

    /**
     * pop one mode (go to previous)
     */
    void pop( bool leave_me = true );

    /**
     * pop until current mode is @arg mode
     */
    void pop( ModeType mode );

    void registerModifierKeys();
    void unregisterModifierKeys();
    void stop();

    YMode* current() const;
    ModeType currentType() const;

private :
    YView* mView;
    QString mKey;
    QString mModifiers;
    YModeMap mModes;
    YModeStack stack;
    int mapMode;
    bool mRegisterKeys;
    bool mStop;
};

#endif // YZ_MODEPOOL_H


