/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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
 * $Id: cursor.cpp 2116 2006-01-14 19:19:01Z panard $
 */

#include <libyzis/drawbuffer.h>

#include "cursor.h"
#include "editor.h"
#include "viewwidget.h"

QYZisCursor::QYZisCursor( QWidget* parent, shape type )
	: QWidget( parent ){
	move( 0, 0 );
	setCursorType( type );
}

QYZisCursor::~QYZisCursor() {
}

QYZisCursor::shape QYZisCursor::type() const {
	return mCursorType;
}
void QYZisCursor::setCursorType( shape type ) {
	if ( type == mCursorType )
		return;
	mCursorType = type;
	int w = parentWidget()->fontMetrics().maxWidth();
	int h = parentWidget()->fontMetrics().lineSpacing();
	if ( mCursorType == VBAR ) 
		w = 2;
	resize( w, h );
}

void QYZisCursor::paintEvent( QPaintEvent* ) {
	QPainter p( this );

#define GET_cell \
	QYZisView* yzview = dynamic_cast<QYZisView*>( parentWidget()->parentWidget() ); \
	YZDrawCell cell = yzview->m_drawBuffer.at( yzview->getCursor() - YZCursor(yzview->getDrawCurrentLeft(),yzview->getDrawCurrentTop()) )
#define SET_pen \
	p.setPen( cell.bg.isValid() ? cell.bg.rgb() : parentWidget()->palette().window().color() );

	switch( type() ) {
		case SQUARE :
			{
			GET_cell;
			SET_pen;
			p.setBackground( cell.fg.isValid() ? QBrush( cell.fg.rgb() ) : parentWidget()->palette().text() );
			p.eraseRect( rect() );
			p.drawText( rect(), cell.c );
			}
			break;
		case RECT :
			{
			GET_cell;
			SET_pen;
			p.drawRect( rect() );
			}
			break;
		case VBAR :
			p.fillRect( rect(), parentWidget()->palette().text() );
			break;
		case HBAR :
			p.fillRect( rect(), parentWidget()->palette().text() );
			break;
	}
}

