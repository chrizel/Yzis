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

/**
 * class YViewCursor
 */
YViewCursor::YViewCursor() : 
	mBuffer(), mColumn(0) {}
YViewCursor::YViewCursor( int line, int position, int column ) : 
	mBuffer(line, position), mColumn(column) {}
YViewCursor::YViewCursor( const YViewCursor &c ) : 
	mBuffer(c.mBuffer), mColumn(c.mColumn) {}
YViewCursor::~YViewCursor() {}

YViewCursor &YViewCursor::operator=( const YViewCursor& c )
{
    mBuffer = c.mBuffer;
	mColumn = c.mColumn;
    return *this;
}

