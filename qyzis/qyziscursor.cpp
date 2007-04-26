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

#include "editor.h"
#include "viewwidget.h"
#include "qyziscursor.h"

#define dbg() yzDebug("QYZisCursor")
#define err() yzError("QYZisCursor")

QYZisCursor::QYZisCursor( QYZisEdit* edit, CursorShape shape ) 
: QWidget( edit )  
{
	mEditor = edit;
	mView = edit->view();

	setAutoFillBackground( false );
	setAttribute( Qt::WA_NoSystemBackground );
	setAttribute( Qt::WA_PaintOnScreen );
	setAttribute( Qt::WA_OpaquePaintEvent );

	move( 0, 0 );
	setCursorShape( shape );
}

QYZisCursor::~QYZisCursor() {
}

QYZisCursor::CursorShape QYZisCursor::shape() const {
	return mCursorShape;
}
void QYZisCursor::setCursorShape( CursorShape shape ) {
	if ( shape == mCursorShape )
		return;
	mCursorShape = shape;
	int w = parentWidget()->fontMetrics().maxWidth();
	int h = parentWidget()->fontMetrics().lineSpacing();
	if ( mCursorShape == CursorVbar ) 
		w = 2;
	resize( w, h );
}

void QYZisCursor::paintEvent( QPaintEvent* pe ) 
{
	Q_UNUSED(pe);

    mView->m_drawBuffer.at( mEditor->translateRealToPosition(pos()) );
	switch( shape() ) {
		case CursorFilledRect : paintFilledRect(); break;
		case CursorRect : paintRect(); break;
        case CursorVbar : paintVbar(); break;
        case CursorHbar: paintHbar(); break;
    }
}

void QYZisCursor::paintFilledRect()
{
	QPainter p( this );
    const YZDrawCell cell( mView->m_drawBuffer.at( mEditor->translateRealToPosition(pos()) ) );
    if (cell.bg.isValid()) {
        dbg() << "paintFilledRect(): valid cell bg" << endl;
        p.setPen( QColor(cell.bg.rgb()) );
    } else {
        dbg() << "paintFilledRect(): invalid cell bg" << endl;
        p.setPen( parentWidget()->palette().color(QPalette::Window) );
    }

    dbg() << "paintFilledRect(): pen=" << p.pen().color().name() << endl;

    if ( cell.fg.isValid() ) {
        dbg() << "paintFilledRect(): valid cell fg" << endl;
        p.setBackground( QColor(cell.fg.rgb()) );
    } else {
        dbg() << "paintFilledRect(): invalid cell fg" << endl;
        p.setBackground( parentWidget()->palette().color(QPalette::WindowText) );
    }

    dbg() << "paintFilledRect(): background=" << p.background().color().name() << endl;

    p.eraseRect( rect() );
    p.drawText( rect(), cell.c );
}

void QYZisCursor::paintRect()
{
	QPainter p( this );
    const YZDrawCell cell( mView->m_drawBuffer.at( mEditor->translateRealToPosition(pos()) ) );
	if (cell.bg.isValid()) {
        dbg() << "paintFilledRect(): valid cell bg" << endl;
        p.setPen( QColor(cell.bg.rgb()) );
    } else {
        dbg() << "paintFilledRect(): invalid cell bg" << endl;
        p.setPen( parentWidget()->palette().color(QPalette::Window) );
    }
    dbg() << "paintFilledRect(): pen=" << p.pen().color().name() << endl;

    p.drawRect( rect() );
}

void QYZisCursor::paintVbar()
{
    QPainter p( this );
    const QBrush b( parentWidget()->palette().text() );
    dbg() << "paintVbar(): brush=" << b.color().name() << endl;
    p.fillRect( rect(), b );
}

void QYZisCursor::paintHbar()
{
	QPainter p( this );
    const QBrush b( parentWidget()->palette().text() );
    dbg() << "paintVbar(): brush=" << b.color().name() << endl;
    p.fillRect( rect(), b );
}

YZDebugStream& QYZisCursor::operator<<( YZDebugStream& out )
{
    switch( mCursorShape ) {
		case QYZisCursor::CursorFilledRect : out << "CursorFilledRect"; break;
		case QYZisCursor::CursorVbar : out << "CursorVbar"; break;
		case QYZisCursor::CursorHbar : out << "CursorHbar"; break;
		case QYZisCursor::CursorRect : out << "CursorRect"; break;
    }
    return out;
}
