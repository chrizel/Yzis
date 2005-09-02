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
 * $Id: cursor.cpp 2072 2005-09-01 11:01:40Z mikmak $
 */

#include "cursor.h"
#include "editor.h"

#include <QPaintEngine>

QYZisCursor::QYZisCursor( QYZisEdit* parent, shape type ) {
	mParent = parent;
	shown = false;
	bg = new QPixmap();
	cursor = new QPixmap();
	setCursorType( type );
	mX = mY = 0;
}

QYZisCursor::~QYZisCursor() {
	delete bg;
	delete cursor;
}

unsigned int QYZisCursor::width() const {
	return bg->width();
}
unsigned int QYZisCursor::height() const {
	return bg->height();
}
QYZisCursor::shape QYZisCursor::type() const {
	return mCursorType;
}

void QYZisCursor::setCursorType( shape type ) {
	if ( shown ) hide();
	mCursorType = type;
	unsigned width = mParent->fontMetrics().maxWidth();
	unsigned height = mParent->fontMetrics().lineSpacing();
	if ( mCursorType == VBAR ) width = 2;
	resize( width, height );
}
void QYZisCursor::resize( unsigned int w, unsigned int h ) {
	if ( shown ) hide();
	bg->resize( w, h );
	cursor->resize( w, h );
}
void QYZisCursor::hide() {
	if ( ! shown ) return;
	QRect rect( mX, mY, bg->width(), bg->height() );
	QPainter p( mParent );
	mParent->erase( rect );
	mParent->drawCell( &p, cell(), rect );
	p.end();
	shown = false;
}
void QYZisCursor::refresh() {
	move( mX, mY );
}
void QYZisCursor::move( unsigned int x, unsigned int y ) {
	if ( shown ) hide();
	mX = x;
	mY = y;
	if ( prepareCursors() ) {
		drawCursor( cursor );
		shown = true;
	}
}

const QYZViewCell& QYZisCursor::cell() const {
	return mParent->mCell[ mY / mParent->fontMetrics().lineSpacing() ][ 
					mParent->mParent->getLocalBooleanOption("rightleft") ? mX + width() : mX ];
}

bool QYZisCursor::prepareCursors() {
	bool ret = true;
	bg->fill( mParent, 0, 0 );
	*cursor = *bg;
	QPainter p( cursor );
	QRect rect = cursor->rect();
	switch( mCursorType ) {
		case SQUARE :
			mParent->drawCell( &p, cell(), rect, true );
			break;
		case RECT :
			p.setPen( mParent->foregroundColor() );
			p.drawRect( rect );
			break;
		case VBAR :
			rect.setRight( 2 );
			p.fillRect( rect, QBrush( mParent->foregroundColor() ) );
			break;
		case HBAR :
			rect.setTop( cursor->height() - 2 );
			p.fillRect( rect, QBrush( mParent->foregroundColor() ) );
			break;
	}
	p.end();
	return ret;
}

void QYZisCursor::drawCursor( QPixmap* orig ) {
/*	if ( mParent->paintingActive() ) {
		mParent->paintEngine()->drawPixmap( QRect( mX, mY, orig->width(), orig->height() ), QPixmap(*orig), orig->rect() );
	} else {*/
		QPainter p( mParent );
		p.drawPixmap( mX, mY, *orig );
//	}
}

