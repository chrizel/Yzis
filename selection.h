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

#ifndef YZ_SELECTION_H
#define YZ_SELECTION_H

#include <qmap.h>

class YZCursor;
class YZView;

struct selection {
	YZCursor * from;
	YZCursor * to;
	YZCursor * drawFrom;
	YZCursor * drawTo;
};
typedef struct selection YZSelection;

typedef QMap<unsigned int, YZSelection> YZSelectionMap;
typedef QMap<QString, YZSelectionMap> YZSelectionLayout;

class YZSelectionPool {

	public:
		YZSelectionPool( YZView * view );
		virtual ~YZSelectionPool( );

		void addSelection( const QString& layout, const YZCursor& from, const YZCursor& to, const YZCursor& drawFrom, const YZCursor& drawTo );
		void addSelection( const QString& layout, unsigned int from_x, unsigned int from_y, unsigned int to_x, unsigned int to_y );
		void delSelection( const QString& layout, const YZCursor& from, const YZCursor& to, const YZCursor& drawFrom, const YZCursor& drawTo );
		void delSelection( const QString& layout, unsigned int from_x, unsigned int from_y, unsigned int to_x, unsigned int to_y );

		void clear( );
		void clear( const QString& layout );

		YZSelectionMap layout( const QString& layout );

		bool isSelected( const YZCursor& pos );

		void debug( const QString& layout );

	private:
		YZView * parentView;
		YZSelectionLayout selectionPool;

		void removeSelection( const QString& layout, unsigned int begin, unsigned int len );
		void insertSelection( const QString& layout, unsigned int pos, const YZCursor& from, const YZCursor& to, const YZCursor& drawFrom, const YZCursor& drawTo );
		int locatePosition( const QString& layout, const YZCursor& pos, bool * isSelected );

};

#endif

