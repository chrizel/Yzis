/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2004-2005 Mickael Marchand <mikmak@yzis.org> 
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

#ifndef YZ_MARK_H
#define YZ_MARK_H

#include "qglobal.h"
#include <QMap>
#include <QString>
#include "cursor.h"
#include "yzismacros.h"

struct YZCursorPos;
class YZCursor;

typedef QMap<QString, YZCursorPos> YZViewMarker;


// struct YZMarkPos{
// 	YZMarkPos(): line(0), col(0)
// 	YZMarkPos(int l, int c): line(l), col(c) {}
// 	uint line;
// 	uint col;
// };

//line <-> marks
typedef QMap<uint, uint> YZDocMarker;


/**Contains view marks (strings)*/
class YZIS_EXPORT YZViewMark {

	public:
		YZViewMark( );
		virtual ~YZViewMark( );

		void clear( );

		void add( const QString& mark, const YZCursor& bPos, const YZCursor& dPos );
		void del( const QString& mark );

		YZCursorPos get( const QString& mark, bool * found );

	private:
		YZViewMarker marker;

};

/**Contains document marks (integers) like KTE bookmarks, breakpoints, etc.*/
class YZIS_EXPORT YZDocMark {

	public:
		YZDocMark( ) {}

		void clear( );

		void add( uint line, uint mark );
		void del( uint line, uint mark );
		void del( uint line );

		uint get( uint line );

		const YZDocMarker &getMarker() { return marker; }

	private:
		YZDocMarker marker;

};

#endif

