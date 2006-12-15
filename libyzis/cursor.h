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

#ifndef YZIS_CURSOR
#define YZIS_CURSOR

#include <QPoint>
#include "yzismacros.h"

class YZDebugStream;

class YZIS_EXPORT YZCursor : public QPoint {

	public :
		YZCursor() : QPoint(-1,-1) { }
		YZCursor( const QPoint& c ) : QPoint( c.x(), c.y() ) { }
		YZCursor( const YZCursor& c ) : QPoint( c.x(), c.y() ) { }
		YZCursor(int x, int y) : QPoint(x,y) { }
		virtual ~YZCursor() {};

		void setXY( int x, int y ) { setX(x); setY(y); }

		/*
		 * operators
		 */
		bool operator< ( const YZCursor& right ) const;
		bool operator<= ( const YZCursor& right ) const;
		bool operator> ( const YZCursor& right ) const;
		bool operator>= ( const YZCursor& right ) const;

};

extern YZIS_EXPORT YZDebugStream &operator<< ( YZDebugStream & out, const YZCursor & c );

struct YZCursorPos {
    YZCursor bPos; /* buffer position */
    YZCursor dPos; /* draw position */
};

#endif

