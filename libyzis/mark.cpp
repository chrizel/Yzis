/*  This file is part of the Yzis libraries
*  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

#include "mark.h"
#include "cursor.h"

#define dbg()    yzDebug("YViewMark")
#define err()    yzError("YViewMark")

void YDocMark::add( uint line, uint mark )
{
    if (marker.contains(line)) {
        mark &= ~marker[line];
        if (mark == 0)
            return ;
        marker[line] |= mark;
    } else
        marker[line] = mark;
}

void YDocMark::del( uint line, uint mark )
{
    mark &= marker[line];
    if (mark == 0)
        return ;
    marker[line] &= ~mark;
    if (marker[line] == 0)
        marker.remove(line);
}

void YDocMark::del( uint line )
{
    marker.remove(line);
}

uint YDocMark::get( uint line ) const
    {
        return marker[line];
    }
