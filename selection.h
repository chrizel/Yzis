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

#include "cursor.h"

class YZCursor;

struct selection {
	YZCursor * from;
	YZCursor * to;
};
typedef struct selection YZSelection;

class YZSelectionPool {

	public:
		YZSelectionPool( YZView * view );
		virtual ~YZSelectionPool( );

		void addSelection( YZCursor * from, YZCursor * to );
		void addSelection( unsigned int from_x, unsigned int from_y, unsigned int to_x, unsigned int to_y );
		void delSelection( YZCursor * from, YZCursor * to );
		void delSelection( unsigned int from_x, unsigned int from_y, unsigned int to_x, unsigned int to_y );

		void clear( );

		bool isSelected( YZCursor * pos );

		void test( ); // XXX test
		void print( ); // XXX test

	private:
		YZView * parentView;
		YZSelection * selectionPool;
		unsigned int size;

		void removeSelection( unsigned int begin, unsigned int len );
		void insertSelection( unsigned int pos, YZCursor * from, YZCursor * to );
		int locatePosition( YZCursor * pos, bool * isSelected );

		void printSelection( unsigned int pos ); // XXX test

};

#endif

