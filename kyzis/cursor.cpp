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

#include "cursor.h"

KYZisCursor::KYZisCursor( KYZisEdit* parent, int type ) {
	mParent = parent;
	shown = false;
	bg = new QPixmap();
	cursor = new QPixmap();
	setCursorType( type );
}

KYZisCursor::~KYZisCursor() {
	delete bg;
	delete cursor;
}

unsigned int KYZisCursor::width() {
	return bg->width();
}
unsigned int KYZisCursor::height() {
	return bg->height();
}

void KYZisCursor::setCursorType( int type ) {
	if ( shown ) hide();
	mCursorType = static_cast<cursorType>(type);
	unsigned width = bg->width();
	unsigned height = bg->height();
	if ( mCursorType == KYZ_CURSOR_LINE ) width = 1;
	resize( width, height );
}
void KYZisCursor::resize( unsigned int w, unsigned int h ) {
	if ( shown ) hide();
	bg->resize( w, h );
	cursor->resize( w, h );
}
void KYZisCursor::hide() {
	if ( ! shown ) return;
	yzDebug() << "KYZisCursor::hide" << endl;
	drawCursor( bg );
	shown = false;
}
void KYZisCursor::refresh() {
	yzDebug() << "KYZisCursor::refresh" << endl;
	move( mX, mY );
}
void KYZisCursor::move( unsigned int x, unsigned int y ) {
	yzDebug() << "KYZisCursor::move" << endl;
	if ( shown ) hide();
	mX = x;
	mY = y;
	prepareCursors();
	drawCursor( cursor );
	shown = true;
}

void KYZisCursor::prepareCursors() {
	bitBlt( bg, 0, 0, mParent, mX, mY, bg->width(), bg->height(), Qt::CopyROP, true );
	bitBlt( cursor, 0, 0, bg );
	QPainter p( cursor );
	switch( mCursorType ) {
		case KYZ_CURSOR_SQUARE :
			bitBlt( cursor, 0, 0, bg, 0, 0, bg->width(), bg->height(), Qt::NotROP, false );
			break;
		case KYZ_CURSOR_LINE :
			p.setPen( mParent->foregroundColor() );
			p.drawLine( 0, 0, 0, cursor->height() );
			break;
		default :
			break;
	}
	p.end();
}

void KYZisCursor::drawCursor( QPixmap* orig ) {
	bitBlt( mParent, mX, mY, orig );
}

