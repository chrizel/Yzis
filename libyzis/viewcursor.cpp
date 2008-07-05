/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>.
*  Copyright (C) 2003-2004 Loic Pauleve <panard@inzenet.org>,
*  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
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

#include "viewcursor.h"
#include "debug.h"

/**
 * class YViewCursor
 */

#define dbg()    yzDebug("YViewCursor")
#define err()    yzError("YViewCursor")

YViewCursor::YViewCursor()
{
    reset();
}

YViewCursor::YViewCursor( const YViewCursor &c )
        : YCursorPos()
{
    copyFields( c );
}

YViewCursor::~YViewCursor( )
{}

void YViewCursor::copyFields( const YViewCursor &c )
{
    mScreen = c.mScreen;
    mBuffer = c.mBuffer;
    lineHeight = c.lineHeight;
}

YViewCursor &YViewCursor::operator=( const YViewCursor& c )
{
    mValid = true;
    copyFields( c );
    return *this;
}

void YViewCursor::reset()
{
    mValid = true;
    lineHeight = 1;
    mBuffer.setX( 0 );
    mBuffer.setY( 0 );
    mScreen.setX( 0 );
    mScreen.setY( 0 );
}

void YViewCursor::invalidate()
{
    mValid = false;
}
bool YViewCursor::valid() const
{
    return mValid;
}

void YViewCursor::debug()
{
    dbg() << "YViewCursor : buffer = " << mBuffer << " ; screen = " << mScreen
    << " ; lineHeight = " << lineHeight << endl
}

int YViewCursor::bufferX() const
{
    return mBuffer.x();
}
int YViewCursor::bufferY() const
{
    return mBuffer.y();
}
int YViewCursor::screenX() const
{
    return mScreen.x();
}
int YViewCursor::screenY() const
{
    return mScreen.y();
}


void YViewCursor::setBufferX( int value )
{
    mBuffer.setX( value );
}
void YViewCursor::setBufferY( int value )
{
    mBuffer.setY( value );
}
void YViewCursor::setScreenX( int value )
{
    mScreen.setX( value );
}
void YViewCursor::setScreenY( int value )
{
    mScreen.setY( value );
}


