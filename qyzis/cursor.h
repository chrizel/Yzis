/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2006 Loic Pauleve <panard@inzenet.org>
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
 * $Id: cursor.h 2116 2006-01-14 19:19:01Z panard $
 */

#ifndef QYZISCURSOR_H
#define QYZISCURSOR_H

#include <QWidget>

class QYZisEdit;

class QYZisCursor : public QWidget {
	Q_OBJECT

	public :
		enum shape {
			SQUARE,
			VBAR,
			HBAR,
			RECT,
		};

		QYZisCursor( QWidget* parent, shape type );
		virtual ~QYZisCursor();

		void setCursorType( shape type );
		shape type() const;

	protected :
		virtual void paintEvent( QPaintEvent* event );

	private :
		shape mCursorType;

};

#endif

