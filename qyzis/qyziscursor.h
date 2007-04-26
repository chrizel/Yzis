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

#ifndef QYZISCURSOR_H
#define QYZISCURSOR_H

#include <QWidget>

#include "debug.h"
#include "drawbuffer.h"

class QYZisEdit;
class QYZisView;

class QYZisCursor : public QWidget
{
	Q_OBJECT

public :
	enum CursorShape {
		CursorFilledRect,
		CursorVbar,
		CursorHbar,
		CursorRect,
	};

	QYZisCursor( QYZisEdit* parent, CursorShape shape );
	virtual ~QYZisCursor();

	void setCursorShape( CursorShape shape );
	CursorShape shape() const;

    YZDebugStream& operator<<( YZDebugStream& out );

protected :
	virtual void paintEvent( QPaintEvent* pe );

    inline void paintFilledRect();
    inline void paintRect();
    inline void paintVbar();
    inline void paintHbar();

private :
	CursorShape mCursorShape;
	QYZisEdit* mEditor;
	QYZisView* mView;

};


#endif

