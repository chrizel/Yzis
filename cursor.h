/* This file is part of the Yzis libraries
 *  Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>
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

#ifndef YZIS_CURSOR
#define YZIS_CURSOR

class YZCursor;
struct YZCursorPos {
	YZCursor* bPos; /* buffer position */
	YZCursor* dPos; /* draw position */
};

#include "debug.h"


class YZView;

/**
 * Handles the cursor information.
 * Maybe we can have more than one cursor per view someday :) ( multiple
 * developers remotely editing the same file :)
 */
class YZCursor {
	
	friend YZDebugStream &operator<< ( YZDebugStream & out, const YZCursor & c );
		
	public :
		YZCursor(YZView *vp);
		YZCursor(YZView *vp, unsigned int X, unsigned int Y );
		YZCursor( const YZCursor& cursor);
		YZCursor(YZCursor *cursor);
		~YZCursor();

		inline void setX(unsigned int x) { x_pos = x; }

		inline void setY(unsigned int y) { y_pos = y; }

		inline unsigned int getX() const { return x_pos; }

		inline unsigned int getY() const { return y_pos; }

		YZView* getParent() { return parentView; }

		void setCursor( YZCursor *cursor );
		void setCursor( const YZCursor& cursor );

		bool operator== ( const YZCursor &right ) const;
		bool operator<= ( const YZCursor &right ) const;
		bool operator>= ( const YZCursor &right ) const;
		bool operator< ( const YZCursor &right ) const;
		bool operator> ( const YZCursor &right ) const;

/*
		inline void incX(int nb=1) { x_pos+=nb; }

		inline void incY(int nb=1) { y_pos+=nb; }

		inline void decX(int nb=1) { x_pos-=nb; }

		inline void decY(int nb=1) { y_pos-=nb; }
*/
	private :
		YZView *parentView;

		/* position relative to the whole file */
		unsigned int x_pos;
		unsigned int y_pos;
};

#endif

