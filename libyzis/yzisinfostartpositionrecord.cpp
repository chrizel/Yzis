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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#include "yzisinfostartpositionrecord.h"

/**
 * YZYzisinfoStartPositionRecord::YZYzsinfoStartPositionRecord
 */

YZYzisinfoStartPositionRecord::YZYzisinfoStartPositionRecord() {
	mFilename = "";
	setPosition( 0, 0 );
}

/**
 * YZYzisinfoStartPositionRecord::YZYzisinfoStartPositionRecord
 */
 
YZYzisinfoStartPositionRecord::YZYzisinfoStartPositionRecord( const QString & filename, const unsigned int x, const unsigned int y ) {
	mFilename = filename;
	setPosition( x, y );
}

/**
 * YZYzisinfoStartPositionRecord::~YZYzsinfoStartPositionRecord
 */
 
YZYzisinfoStartPositionRecord::~YZYzisinfoStartPositionRecord() {
	delete mPosition;
}

/**
 * YZYzisinfoStartPositionRecord::YZYzsinfoStartPositionRecord
 */

YZYzisinfoStartPositionRecord::YZYzisinfoStartPositionRecord( YZYzisinfoStartPositionRecord & copy ) {
	mFilename = copy.filename();
	setPosition( copy.position()->x(), copy.position()->y() );
}

/**
 * YZYzisinfoStartPositionRecord::operator=
 */
 
YZYzisinfoStartPositionRecord & YZYzisinfoStartPositionRecord::operator=( YZYzisinfoStartPositionRecord & copy ) {
	if ( this == &copy ) {
		return *this;
	}
	
	mFilename = copy.filename();
	setPosition( copy.position()->x(), copy.position()->y() );
	
	return *this;
}

/**
 * YZYzisinfoStartPositionRecord::filename
 */
 
QString & YZYzisinfoStartPositionRecord::filename() {
	return mFilename;
}

/**
 * YZYzisinfoStartPositionRecord::position
 */
 
YZCursor * YZYzisinfoStartPositionRecord::position() {
	return mPosition;
}

/**
 * YZYzisinfoStartPositionRecord::setFilename
 */
 
void YZYzisinfoStartPositionRecord::setFilename( const QString & filename ) {
	mFilename = filename;
}

/**
 * YZYzisinfoStartPositionRecord::setPosition
 */
 
void YZYzisinfoStartPositionRecord::setPosition( const unsigned int x, const unsigned int y ) {
	mPosition = new YZCursor( x, y );
}

/*
 * END OF FILE
 */
