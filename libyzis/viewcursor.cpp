/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>.
 *  Copyright (C) 2003-2004 Loic Pauleve <panard@inzenet.org>,
 *  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
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

#include "viewcursor.h"
#include "debug.h"

/**
 * class YZViewCursor
 */

YZViewCursor::YZViewCursor( YZView* parent ) {
	mParent = parent;
	reset();
}

YZViewCursor::YZViewCursor( const YZViewCursor &c ) {
    copyFields( c );
}

YZViewCursor::~YZViewCursor( ) {
}

void YZViewCursor::copyFields( const YZViewCursor &c )
{
	mScreen = c.mScreen;
	mBuffer = c.mBuffer;
	spaceFill = c.spaceFill;
	bColIncrement = c.bColIncrement;
	bLineIncrement = c.bLineIncrement;
	sColIncrement = c.sColIncrement;
	sLineIncrement = c.sLineIncrement;
	lineHeight = c.lineHeight;
	lastCharWasTab = c.lastCharWasTab;
	wrapTab = c.wrapTab;
	wrapNextLine = c.wrapNextLine;
}

YZViewCursor &YZViewCursor::operator=( const YZViewCursor& c ) {
	mValid = true;
    copyFields( c );
	return *this;
}

void YZViewCursor::reset() {
	mValid = true;
	spaceFill = 0;
	bColIncrement = 1; // XXX seems to be no longer used... ( always 1 )
	bLineIncrement = 0;
	sColIncrement = 1;
	sLineIncrement = 0;
	lineHeight = 1;
	lastCharWasTab = false;
	wrapNextLine = false;
	wrapTab = false;
	mBuffer.setX( 0 );
	mBuffer.setY( 0 );
	mScreen.setX( 0 );
	mScreen.setY( 0 );
}

void YZViewCursor::invalidate() {
	mValid = false;
}
bool YZViewCursor::valid() const {
	return mValid;
}

void YZViewCursor::debug() {
	yzDebug() << "YZViewCursor : buffer = " << mBuffer << " ; screen = " << mScreen
		<< " ; wrapNextLine = " << wrapNextLine << " ; wrapTab = " << wrapTab << endl
		<< "               bLineIncrement = " << bLineIncrement << "; sLineIncrement = " << sLineIncrement
		<< " ; lineHeight = " << lineHeight << endl
		<< "               lastCharWasTab = " << lastCharWasTab << " ; sColIncrement = " << sColIncrement << endl;
}

int YZViewCursor::bufferX() const {
	return mBuffer.x();
}
int YZViewCursor::bufferY() const {
	return mBuffer.y();
}
int YZViewCursor::screenX() const {
	return mScreen.x();
}
int YZViewCursor::screenY() const {
	return mScreen.y();
}


void YZViewCursor::setBufferX( int value ) {
	mBuffer.setX( value );
}
void YZViewCursor::setBufferY( int value ) {
	mBuffer.setY( value );
}
void YZViewCursor::setScreenX( int value ) {
	mScreen.setX( value );
}
void YZViewCursor::setScreenY( int value ) {
	mScreen.setY( value );
}


