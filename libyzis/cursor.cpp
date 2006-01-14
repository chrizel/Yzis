/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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

/**
 * $Id$
 */

#include "cursor.h"

YZCursor::YZCursor() 
    : x_pos( 0 )
    , y_pos( 0 )
{
}
YZCursor::YZCursor( unsigned int x, unsigned int y )
    : x_pos( x )
    , y_pos( y )
{
}

YZCursor::YZCursor( const YZCursor& c) 
    : x_pos( c.x_pos )
    , y_pos( c.y_pos )
{
}

YZCursor &YZCursor::operator=( const YZCursor &rhs )
{
    x_pos = rhs.x_pos;
    y_pos = rhs.y_pos;

    return *this;
}

bool operator==( const YZCursor &lhs, const YZCursor &rhs )
{
    return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

bool operator<( const YZCursor &lhs, const YZCursor &rhs )
{
	return ( lhs.y() < rhs.y() || lhs.y() == rhs.y() && lhs.x() < rhs.x() );
}
const YZCursor operator-( const YZCursor& l, const YZCursor& r ) {
	return YZCursor( l.x() - r.x(), l.y() - r.y() );
}

YZDebugStream &operator<< ( YZDebugStream &out, const YZCursor & c ) {
	out << "(" << c.x() << "," << c.y() << ")";
	return out;
}

