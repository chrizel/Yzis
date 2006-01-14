/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Loic Pauleve <panard@inzenet.org>
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

/**
 * $Id$
 */

#ifndef KYZISCURSOR_H
#define KYZISCURSOR_H

//#include "editor.h"
#include <qpixmap.h>
#include <QWidget>

class KYZisEdit;

class KYZisCursor : public QWidget {
	Q_OBJECT

	public :
		enum shape {
			SQUARE,
			VBAR,
			HBAR,
			RECT,
		};

		KYZisCursor( KYZisEdit* parent, shape type );
		virtual ~KYZisCursor();

		void setCursorType( shape type );
		shape type() const;

/*		void resize( unsigned int w, unsigned int h );
		void move( unsigned int x, unsigned int y, QPainter* p = NULL );
		void hide(QPainter*parentPainter=NULL);
		void refresh( QPainter* p = NULL );

		unsigned int width() const;
		unsigned int height() const;

		inline unsigned int x() const { return mX; }
		inline unsigned int y() const { return mY; }
		inline unsigned int visible() const { return shown; }
*/
	protected :
		virtual void paintEvent( QPaintEvent* event );

	private :
		shape mCursorType;

};

#endif

