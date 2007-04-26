/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Adam Connell <adam@argoncorp.com>,
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

#include "cursor.h"
#include "view.h"
#include "debug.h"
#include "buffer.h"
#include "linesearch.h"

#define dbg()    yzDebug("YZLineSearch")
#define err()    yzError("YZLineSearch")

/**
 * class YZLineSearch
 */
YZLineSearch::YZLineSearch( const YZView *_view ) {
	dbg() << "YZLineSearch Constructor" << endl;
 	mView = _view;
	mFirstTime = true;
	//mPrevSearched = "a";
}

YZLineSearch::~YZLineSearch() {
//        dbg() << "YZLineSearch Destructor" << endl;
}

YZCursor YZLineSearch::forward( const QString& ch, bool& found, unsigned int times ) {
	YZCursor cur = mView->getBufferCursor();
	int x = cur.x() + 1; // Start search after cursor pos
	int y = cur.y();
	const QString& current = mView->myBuffer()->textline( y );
	int index = 0;
	unsigned int nfound = 0;
	while ( nfound < times && x < current.length() ) {
		index = current.indexOf( ch, x );
		if ( index < 0 )
			break;
		x = index + 1;
		nfound++;
	}
	YZCursor pos;
	found = ( nfound == times );
	if ( found ) {
		pos.setX( x - 1 );
		pos.setY( y );
	}
	updateHistory( ch, YZ_LINE_SEARCH_FORWARD );
	return pos;
}

YZCursor YZLineSearch::forwardBefore( const QString& ch, bool& found, unsigned int times ) {
	YZCursor pos = forward( ch, found, times );
	if ( found ) {
		pos.setX( pos.x() - 1 );
	}
	updateHistory( ch, YZ_LINE_SEARCH_FBEFORE );
	return pos;
}

YZCursor YZLineSearch::reverse( const QString& ch, bool& found, unsigned int times ) {
	YZCursor cur = mView->getBufferCursor();
	unsigned int x = cur.x();
	unsigned int y = cur.y();
	if ( x ) x--; // Start search before current cursor
	const QString& current = mView->myBuffer()->textline( y );
	int index = 0;
	unsigned int nfound = 0;
	while ( nfound < times && x > 0 ) {
		index = current.lastIndexOf( ch, x );
		if ( index < 0 )
			break;
		x = index - 1;
		nfound++;
	}
	YZCursor pos;
	found = ( nfound == times );
	if ( found ) {
		pos.setX( x + 1 );
		pos.setY( y );
	}
	updateHistory( ch, YZ_LINE_SEARCH_REVERSE );
	return pos;
}
	
YZCursor YZLineSearch::reverseAfter( const QString& ch, bool& found, unsigned int times ) {
	YZCursor pos = reverse( ch, found, times );
	if ( found ) {
		pos.setX( pos.x() + 1 );
	}
	updateHistory( ch, YZ_LINE_SEARCH_RAFTER );
	return pos;
}

YZCursor YZLineSearch::searchAgain( bool &found, unsigned int times ) {
	YZCursor garbage;
	found = false;
	if ( mFirstTime ) {
		// Can't search again if we haven't searched a first time...
		dbg() << "Haven't searched before" << endl;
		return garbage;
	}
	
	dbg() << "Searching for: " << mPrevSearched << endl;
	switch ( mType ) {
	case YZ_LINE_SEARCH_FORWARD:
		return forward( mPrevSearched, found, times );
	case YZ_LINE_SEARCH_FBEFORE:
		return forwardBefore( mPrevSearched, found, times );
	case YZ_LINE_SEARCH_REVERSE:
		return reverse( mPrevSearched, found, times ); 
	case YZ_LINE_SEARCH_RAFTER:
		return reverseAfter( mPrevSearched, found, times );
	default:
		/* Can't happen */
		dbg() << "Invalid line search type" << endl;
		return garbage;
	}
}

YZCursor YZLineSearch::searchAgainOpposite( bool &found, unsigned int times ) {
	switch ( mType ) {
	case YZ_LINE_SEARCH_FORWARD:
		mType = YZ_LINE_SEARCH_REVERSE;
		break;
	case YZ_LINE_SEARCH_FBEFORE:
		mType = YZ_LINE_SEARCH_RAFTER;
		break;
	case YZ_LINE_SEARCH_REVERSE:
		mType = YZ_LINE_SEARCH_FORWARD;
		break;
	case YZ_LINE_SEARCH_RAFTER:
		mType = YZ_LINE_SEARCH_FBEFORE;
		break;
	}
	return searchAgain( found, times );
}

/* PRIVATE */
void YZLineSearch::updateHistory( const QString& newch, searchType type ) {
	mPrevSearched = newch;
	mType = type;
	mFirstTime = false;
}

