/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
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

#ifndef YZ_VIEWCURSOR_H
#define YZ_VIEWCURSOR_H

#include "cursor.h"
class YZView;

class YZView;

/**
 * $Id$
 */

/**
 * class YZViewCursor : buffer and screen cursor with all members that YZView needs to move it.
 * this is only an interface, it doesn't have to know how move itself ( this is YZView stuff )
 */
class YZViewCursor {

	friend class YZView;

	public:
		explicit YZViewCursor( YZView* parent );
		YZViewCursor( const YZViewCursor &c);
		virtual ~YZViewCursor();

		YZViewCursor &operator=( const YZViewCursor &c );

		void reset();

		unsigned int bufferX() const;
		unsigned int bufferY() const;
		unsigned int screenX() const;
		unsigned int screenY() const;

		// toggle valid token to false
		void invalidate();

		bool valid() const;

		inline const YZCursor &buffer() const {
			return mBuffer;
		}
		inline const YZCursor &screen() const {
			return mScreen;
		}

		/**
		 * curLineHeight : line height at current cursor position
		 */
		inline unsigned int curLineHeight() const {
			return lineHeight;
		}

		void debug();

		void setBuffer( const YZCursor& value );
		void setScreen( const YZCursor& value );

		void setBufferX( unsigned int value );
		void setBufferY( unsigned int value );
		void setScreenX( unsigned int value );
		void setScreenY( unsigned int value );

	private :
        void copyFields( const YZViewCursor &rhs );
        
		/**
		 * parent view
		 */
		YZView* mParent;

		/**
		 * buffer cursor
		 */
		YZCursor mBuffer;

		/**
		 * screen cursor
		 */
		YZCursor mScreen;

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

		/**
		 * valid token
		 */
		bool mValid;

};

#endif
