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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

/**
 * $Id$
 */

#include "cursor.h"
#include "editor.h"

#include <QPaintEngine>

KYZisCursor::KYZisCursor( KYZisEdit* parent, shape type ) {
	mParent = parent;
	shown = false;
	bg = new QPixmap();
	cursor = new QPixmap();
	setCursorType( type );
	mX = mY = 0;
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
	if ( mCursorType == VBAR ) width = 2;
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
void KYZisCursor::refresh( QPainter* p ) {
	move( mX, mY, p );
}
void KYZisCursor::move( unsigned int x, unsigned int y, QPainter* p ) {
	if ( shown ) hide();
	mX = x;
	mY = y;
	if ( prepareCursors() ) {
		drawCursor( cursor, p );
		shown = true;
	}
}

const KYZViewCell& KYZisCursor::cell() const {
	return mParent->mCell[ mY / mParent->fontMetrics().lineSpacing() ][ 
					mParent->mParent->getLocalBooleanOption("rightleft") ? mX + width() : mX ];
}

bool KYZisCursor::prepareCursors() {
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

void KYZisCursor::drawCursor( QPixmap* orig, QPainter* p ) {
	bool isPainting = ( p != NULL );
	if ( !isPainting ) {
		if ( mParent->paintingActive() ) {
			yzDebug() << "KYZisCursor::drawCursor !!! paintingActive but no painter provided !!! - aborting" << endl;
			return;
		}
		p = new QPainter();
		p->begin( mParent );
	}
	p->drawPixmap( mX, mY, *orig );
	if ( !isPainting ) {
		p->end();
		delete p;
	}
}

