/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <marchand@kde.org>
 *  Thomas Capricelli <orzel@freehackers.org>.
 *  Loic Pauleve <panard@inzenet.org>
 *  Pascal "Poizon" Maillard <poizon@gmx.at>
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

#include "viewcursor.h"

/**
 * class YZViewCursor
 */

YZViewCursor::YZViewCursor( YZView* parent ) {
	mParent = parent;
	mBuffer = new YZCursor( mParent );
	mScreen = new YZCursor( mParent );
	reset();
}

YZViewCursor::YZViewCursor( const YZViewCursor &c ) {
	mBuffer = new YZCursor( c.mParent );
	mScreen = new YZCursor( c.mParent );
	*this = c;
}

YZViewCursor::~YZViewCursor( ) {
	delete mBuffer;
	delete mScreen;
}

YZViewCursor &YZViewCursor::operator=( const YZViewCursor& c ) {
	*mScreen = *c.mScreen;
	*mBuffer = *c.mBuffer;
	spaceFill = c.spaceFill;
	bColIncrement = c.bColIncrement;
	bLineIncrement = c.bLineIncrement;
	sColIncrement = c.sColIncrement;
	sLineIncrement = c.sLineIncrement;
	lineHeight = c.lineHeight;
	lastCharWasTab = c.lastCharWasTab;
	wrapTab = c.wrapTab;
	wrapNextLine = c.wrapNextLine;
	return *this;
}

void YZViewCursor::reset() {
	spaceFill = 0;
	bColIncrement = 1; // XXX seems to be no longer used... ( always 1 )
	bLineIncrement = 0;
	sColIncrement = 1;
	sLineIncrement = 0;
	lineHeight = 1;
	wrapNextLine = false;
	wrapTab = false;
	mBuffer->setX( 0 );
	mBuffer->setY( 0 );
	mScreen->setX( 0 );
	mScreen->setY( 0 );
}

void YZViewCursor::debug() {
	yzDebug()	<< "YZViewCursor : buffer = " << *mBuffer << " ; screen = " << *mScreen << endl
		  	<< "               bLineIncrement = " << bLineIncrement << "; sLineIncrement = " << sLineIncrement << endl
			<< "               lineHeight = " << lineHeight << " ; wrapNextLine = " << wrapNextLine << endl;
}

unsigned int YZViewCursor::bufferX() const {
	return mBuffer->getX();
}
unsigned int YZViewCursor::bufferY() const {
	return mBuffer->getY();
}
unsigned int YZViewCursor::screenX() const {
	return mScreen->getX();
}
unsigned int YZViewCursor::screenY() const {
	return mScreen->getY();
}

void YZViewCursor::setBuffer( const YZCursor& value ) {
	mBuffer->setCursor( value );
}
void YZViewCursor::setScreen( const YZCursor& value ) {
	mScreen->setCursor( value );
}
void YZViewCursor::setBufferX( unsigned int value ) {
	mBuffer->setX( value );
}
void YZViewCursor::setBufferY( unsigned int value ) {
	mBuffer->setY( value );
}
void YZViewCursor::setScreenX( unsigned int value ) {
	mScreen->setX( value );
}
void YZViewCursor::setScreenY( unsigned int value ) {
	mScreen->setY( value );
}


