/* This file is part of the Yzis libraries
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

#ifndef KYZISCURSOR_H
#define KYZISCURSOR_H

#include "editor.h"

class KYZisEdit;

class KYZisCursor {

	public :

		enum cursorType {
			KYZ_CURSOR_SQUARE = 0,
			KYZ_CURSOR_LINE,
		} mCursorType;

		KYZisCursor( KYZisEdit* parent, int type );
		virtual ~KYZisCursor();

		void setCursorType( int type );
		void resize( unsigned int w, unsigned int h );
		void move( unsigned int x, unsigned int y );
		void hide();
		void refresh();

		inline unsigned int x() { return mX; }
		inline unsigned int y() { return mY; }
		inline unsigned int visible() { return shown; }

	private :

		void drawCursor( QPixmap* orig );
		void prepareCursors();

		KYZisEdit* mParent;

		QPixmap* bg;
		QPixmap* cursor;

		unsigned int mX;
		unsigned int mY;
		bool shown;

};

#endif

