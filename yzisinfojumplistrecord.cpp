/* This file is part of the Yzis libraries
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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

#include "yzisinfojumplistrecord.h"

/**
 * YZYzisinfoJumpListRecord::YZYzisinfoJumpListRecord
 */

YZYzisinfoJumpListRecord::YZYzisinfoJumpListRecord() {
}

/**
 * YZYzisinfoJumpListRecord::YZYzisinfoJumpListRecord
 */
 
YZYzisinfoJumpListRecord::YZYzisinfoJumpListRecord( const QString & filename, const unsigned int x, const unsigned int y ) {
	mFilename = filename;
	setPosition( x, y );
}

/**
 * YZYzisinfoJumpListRecord::~YZYzisinfoJumpListRecord
 */
 
YZYzisinfoJumpListRecord::~YZYzisinfoJumpListRecord() {
}

/**
 * YZYzisinfoJumpListRecord::YZYzisinfoJumpListRecord
 */
 
YZYzisinfoJumpListRecord::YZYzisinfoJumpListRecord( const YZYzisinfoJumpListRecord & copy ) {
	mFilename = copy.filename();
	mPosition = YZCursor( copy.position().x(), copy.position().y() );
}

/**
 * YZYzisinfoJumpListRecord::operator=
 */
 
YZYzisinfoJumpListRecord & YZYzisinfoJumpListRecord::operator=( const YZYzisinfoJumpListRecord & copy ) {
	mFilename = copy.filename();
	mPosition = YZCursor( copy.position().x(), copy.position().y() );
	
	return *this;
}

/**
 * YZYzisinfoJumpListRecord::filename
 */
 
QString & YZYzisinfoJumpListRecord::filename() {
	return mFilename;
}

/**
 * YZYzisinfoJumpListRecord::position
 */
 
YZCursor  YZYzisinfoJumpListRecord::position() {
	return mPosition;
}

/**
 * YZYzisinfoJumpListRecord::filename
 */
 
const QString & YZYzisinfoJumpListRecord::filename() const {
	return mFilename;
}

/**
 * YZYzisinfoJumpListRecord::position
 */
 
const YZCursor  YZYzisinfoJumpListRecord::position() const {
	return mPosition;
}

/**
 * YZYzisinfoJumpListRecord::previousSearchPosition
 */
 
/*YZCursor * YZYzisinfoJumpListRecord::previousSearchPosition() {
	
	--mIndex;
	
	if ( mIndex < 0 ) {
		mIndex = mCursorData.count() - 1;
	}
	
	return mCursorData[mIndex];
}*/

/**
 * YZYzisinfoJumpListRecord::setFilename
 */
 
void YZYzisinfoJumpListRecord::setFilename( const QString & filename ) {
	mFilename = filename;
}

/**
 * YZYzisinfoJumpListRecord::setPosition
 */

void YZYzisinfoJumpListRecord::setPosition( const unsigned int x, const unsigned int y ) {
	mPosition = YZCursor( x, y );
}

/*
 * END OF FILE
 */
