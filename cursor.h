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

#ifndef YZIS_CURSOR
#define YZIS_CURSOR

#include "debug.h"
/**
 * Simple cursor
 */
class YZCursor {
public :
    YZCursor();
    explicit YZCursor( unsigned int x, unsigned int y );
    YZCursor( const YZCursor& cursor );

    YZCursor &operator=( const YZCursor &rhs );

    inline void setX(unsigned int x) { x_pos = x; }
    inline void setY(unsigned int y) { y_pos = y; }

    inline unsigned int x() const { return x_pos; }
    inline unsigned int y() const { return y_pos; }

private :
    unsigned int x_pos;
    unsigned int y_pos;
};

extern bool operator==( const YZCursor &lhs, const YZCursor &rhs );
extern bool operator< ( const YZCursor &lhs, const YZCursor &rhs );

inline bool operator<=( const YZCursor &lhs, const YZCursor &rhs ) { return lhs == rhs || lhs < rhs; }
inline bool operator> ( const YZCursor &lhs, const YZCursor &rhs ) { return !(lhs == rhs) && !(lhs < rhs); }
inline bool operator>=( const YZCursor &lhs, const YZCursor &rhs ) { return !(lhs < rhs); }
inline bool operator!=( const YZCursor &lhs, const YZCursor &rhs ) { return !(lhs == rhs); }

extern YZDebugStream &operator<< ( YZDebugStream & out, const YZCursor & c );

struct YZCursorPos {
    YZCursor* bPos; /* buffer position */
    YZCursor* dPos; /* draw position */
};

#endif

