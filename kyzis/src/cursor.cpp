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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#include "cursor.h"
#include "editor.h"

KYZisCursor::KYZisCursor( KYZisEdit* parent, shape type ) {
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

unsigned int KYZisCursor::width() const {
	return bg->width();
}
unsigned int KYZisCursor::height() const {
	return bg->height();
}
KYZisCursor::shape KYZisCursor::type() const {
	return mCursorType;
}

void KYZisCursor::setCursorType( shape type ) {
	if ( shown ) hide();
	mCursorType = type;
	unsigned width = mParent->fontMetrics().maxWidth();
	unsigned height = mParent->fontMetrics().lineSpacing();
	if ( mCursorType == VBAR ) width = 1;
	resize( width, height );
}
void KYZisCursor::resize( unsigned int w, unsigned int h ) {
	if ( shown ) hide();
	bg->resize( w, h );
	cursor->resize( w, h );
}
void KYZisCursor::hide() {
	if ( ! shown ) return;
	QRect rect( mX, mY, bg->width(), bg->height() );
	QPainter p( mParent );
	mParent->erase( rect );
	mParent->drawCell( &p, cell(), rect );
	p.end();
	shown = false;
}
void KYZisCursor::refresh() {
	move( mX, mY );
}
void KYZisCursor::move( unsigned int x, unsigned int y ) {
	if ( shown ) hide();
	mX = x;
	mY = y;
	if ( prepareCursors() ) {
		drawCursor( cursor );
		shown = true;
	}
}

const KYZViewCell& KYZisCursor::cell() const {
	return mParent->mCell[ mY / mParent->fontMetrics().lineSpacing() ][ 
					mParent->mParent->getLocalBooleanOption("rightleft") ? mX + width() : mX ];
}

bool KYZisCursor::prepareCursors() {
	bool ret = true;
	bitBlt( bg, 0, 0, mParent, mX, mY, bg->width(), bg->height(), Qt::CopyROP, true );
	bitBlt( cursor, 0, 0, bg );
	QPainter p( cursor );
	switch( mCursorType ) {
		case SQUARE :
		{
			QRect rect( 0, 0, cursor->width(), cursor->height() );
			mParent->drawCell( &p, cell(), rect, true );
			break;
		}
		case VBAR :
			p.setPen( mParent->foregroundColor() );
			p.drawLine( 0, 0, 0, cursor->height() - 1);
			break;
		case HBAR :
			p.setPen( mParent->foregroundColor() );
			p.drawLine( 0, cursor->height() - 1, cursor->width() - 1, cursor->height() - 1 );
			break;
	}
	p.end();
	return ret;
}

void KYZisCursor::drawCursor( QPixmap* orig ) {
	bitBlt( mParent, mX, mY, orig );
}

