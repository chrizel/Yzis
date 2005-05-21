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

#include "yzisinforecordsearchhistory.h"

YZYzisinfoRecordSearchHistory::YZYzisinfoRecordSearchHistory() {
	
	mIndex = 0;
	mKeyword = "";
	mFilename = "";
}

YZYzisinfoRecordSearchHistory::YZYzisinfoRecordSearchHistory(YZYzisinfoRecordSearchHistory & copy) 
	: YZYzisinfoRecord() {
		
	mKeyword = copy.keyword();
	mFilename = copy.filename();
	mIndex = copy.mIndex;
	//mCursorData = copy.mCursorData;
}

YZYzisinfoRecordSearchHistory::~YZYzisinfoRecordSearchHistory(){
}

YZYzisinfoRecordSearchHistory & YZYzisinfoRecordSearchHistory::operator=(YZYzisinfoRecordSearchHistory & copy) {
		
	if ( this == &copy ) {
		return *this;
	}
	
	mKeyword = copy.keyword();
	mFilename = copy.filename();
	mIndex = copy.mIndex;
	
	return *this;
}

YZYzisinfoRecordSearchHistory::YZYzisinfoRecordSearchHistory(const QString & keyword, const QString & filename, const unsigned int x, const unsigned int y) {
	
	mIndex = 0;
	mKeyword = keyword;
	mFilename = filename;
	setPosition( x, y );
}

QString & YZYzisinfoRecordSearchHistory::filename() {
	return mFilename;
}

YZCursor * YZYzisinfoRecordSearchHistory::position() {
	return mCursorData[mIndex];
}

YZCursor * YZYzisinfoRecordSearchHistory::previousSearchPosition() {
	
	--mIndex;
	
	if ( mIndex < 0 ) {
		mIndex = mCursorData.count() - 1;
	}
	
	return mCursorData[mIndex];
}

void YZYzisinfoRecordSearchHistory::setFilename( const QString & filename ) {
	mFilename = filename;
}

void YZYzisinfoRecordSearchHistory::setPosition( const unsigned int x, const unsigned int y ) {
	
	bool found = false;
	
	for ( QValueVector<YZCursor*>::Iterator it = mCursorData.begin(); it != mCursorData.end(); ++it ) {
		if ( ( (*it)->x() == x ) && ( (*it)->y() == y ) ) {
			found = true;
			break;
		}
	}
	
	if ( ! found ) {
		mCursorData.push_back( new YZCursor( x, y ) );
	}
	
	return;
}
