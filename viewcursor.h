/* This file is part of the Yzis libraries
 *  Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>,
 *  Thomas Capricelli <orzel@freehackers.org>
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

#ifndef YZ_VIEWCURSOR_H
#define YZ_VIEWCURSOR_H

#include "cursor.h"

/**
 * $Id$
 */

/**
 * class YZViewCursor : buffer and screen cursor with all members that YZView needs to move it.
 * this is only an interface, it doesn't have to know how move itself ( this is YZView stuff )
 */
class YZViewCursor {

	friend class YZView;

	public :
		YZViewCursor( YZView* parent );
		YZViewCursor( const YZViewCursor &c);
		virtual ~YZViewCursor();

		void reset();

		unsigned int bufferX() const;
		unsigned int bufferY() const;
		unsigned int screenX() const;
		unsigned int screenY() const;
		
		inline YZCursor* buffer() {
			return mBuffer;
		}
		inline YZCursor* screen() {
			return mScreen;
		}

		YZViewCursor &operator=( const YZViewCursor &c );

		void debug();

	private :
		void setBuffer( const YZCursor& value );
		void setScreen( const YZCursor& value );

		void setBufferX( unsigned int value );
		void setBufferY( unsigned int value );
		void setScreenX( unsigned int value );
		void setScreenY( unsigned int value );

		/**
		 * parent view
		 */
		YZView* mParent;

		/**
		 * buffer cursor
		 */
		YZCursor* mBuffer;
		
		/**
		 * screen cursor
		 */
		YZCursor* mScreen;

		/**
		 * spaceFill is the shift for starting tabs 
		 * ( when wrapping a line, or scrolling horizontally )
		 */
		unsigned int spaceFill;

		/**
		 * buffer column increment
		 */
		unsigned int bColIncrement;

		/**
		 * buffer line increment
		 */
		unsigned int bLineIncrement;

		/**
		 * screen column increment
		 */
		unsigned int sColIncrement;

		/**
		 * screen line increment
		 */
		unsigned int sLineIncrement;

		/**
		 * current line height
		 */
		unsigned int lineHeight;

		/**
		 * last char was a tab ?
		 */
		bool lastCharWasTab;

		/**
		 * are we wrapping a tab ?
		 */
		bool wrapTab;

		/**
		 * are we wrapping a line ?
		 */
		bool wrapNextLine;

};

#endif
