/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#ifndef YZ_MARK_H
#define YZ_MARK_H

#include <qstring.h>
#include <qmap.h>

#include "cursor.h"

class YZCursor;

typedef QMap<QString, YZCursorPos> YZMarker;

class YZMark {

	public:
		YZMark( );
		virtual ~YZMark( );

		void clear( );

		void add( const QString& mark, const YZCursor& bPos, const YZCursor& dPos );
		void del( const QString& mark );

		YZCursorPos get( const QString& mark, bool * found );

	private:
		YZMarker marker;

};

#endif

