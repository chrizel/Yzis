/*  This file is part of the Yzis libraries
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

#include "search.h"

#include "session.h"
#include "buffer.h"
#include "selection.h"
#include "action.h"

YZSearch::YZSearch() {
	mCurrentSearch = QString::null;
}

YZSearch::~YZSearch() {
}

YZCursor YZSearch::forward( YZView* mView, const QString& pattern, bool* found ) {
	return doSearch( mView, pattern, false, false, false, found );
}
YZCursor YZSearch::backward( YZView* mView, const QString& pattern, bool* found ) {
	return doSearch( mView, pattern, true, false, false, found );
}
YZCursor YZSearch::replayForward( YZView* mView, bool* found, bool skipline ) {
	return doSearch( mView, mCurrentSearch, false, true, skipline, found );
}
YZCursor YZSearch::replayBackward( YZView* mView, bool* found, bool skipline ) {
	return doSearch( mView, mCurrentSearch, true, true, skipline, found );
}

const QString& YZSearch::currentSearch() const {
	return mCurrentSearch;
}

YZCursor YZSearch::doSearch( YZView* mView, const QString& pattern, bool reverse, bool replay, bool skipline, bool* found ) {
	yzDebug() << "YZSearch::doSearch " << pattern << ", " << reverse << ", " << replay << endl;
	*found = false;
	setCurrentSearch( pattern );
	int direction = reverse ? 0 : 1;

	YZCursor ret = mView->getBufferCursor();
	if ( replay ) {
		if ( skipline ) {
			ret.setX( 0 );
			if ( ! reverse ) ret.setY( QMIN( ret.getY() + 1, mView->myBuffer()->lineCount() - 1 ) );
		} else {
			ret.setX( QMAX( ret.getX() + direction, 0 ) );
		}
	}

	YZCursor end( mView );
	if ( reverse ) {
		end.setX( 0 );
		end.setY( 0 );
	} else {
		end.setY( mView->myBuffer()->lineCount() - 1 );
		end.setX( QMAX( mView->myBuffer()->textline( end.getY() ).length() - 1, 0 ) );
	}

	unsigned int matchedLength;
	yzDebug() << "begin = " << ret << endl;
	ret = mView->myBuffer()->action()->search( mView, pattern, ret, end, reverse, &matchedLength, found );
	yzDebug() << "ret = " << ret << endl;

	return ret;
}

void YZSearch::setCurrentSearch( const QString& pattern ) {
	if ( mCurrentSearch == pattern ) return;
	mCurrentSearch = pattern;

	YZBufferMap buffers = YZSession::me->buffers();
	YZBufferMap::Iterator it = buffers.begin(), it_end = buffers.end();

	YZSelectionMap searchMap;
	for( ; it != it_end; it++ ) {
		YZBuffer* b = it.data();
		QPtrList<YZView> views = b->views();

		searchMap.clear();

		/** search all **/
		bool doIt = false;
		for( YZView* v = views.first(); ! doIt && v ; v = views.next() ) {
			doIt = doIt || v->getLocalBoolOption( "hlsearch" );
		}
		if ( doIt ) {
			YZView* v = views.first();
			YZCursor from( v, 0, 0 );
			YZCursor cur( from );
			YZCursor end( v );
			end.setY( b->lineCount() - 1 );
			end.setX( QMAX( b->textline( end.getY() ).length() - 1, 0 ) );

			bool found = true;
			unsigned int matchedLength = 0;
			unsigned int pos = 0;
			do {
				from = b->action()->search( v, mCurrentSearch, cur, end, false, &matchedLength, &found );
				if ( found && matchedLength > 0 ) {
					cur.setCursor( from );
					cur.setX( cur.getX() + matchedLength - 1 );
					YZSelection sel( from, cur, from, cur );
					searchMap.insert( pos++, sel );
				}
			} while ( found );
		}

		for( YZView* v = views.first(); v; v = views.next() ) {
			highlightSearch( v, searchMap );
		}
	}
}

void YZSearch::highlightSearch( YZView* mView, YZSelectionMap searchMap ) {
	YZSelectionPool* pool = mView->getSelectionPool();
	bool wasEmpty = pool->layout( "SEARCH" ).isEmpty();

	pool->clear( "SEARCH" );
	if ( mView->getLocalBoolOption( "hlsearch" ) ) {
		pool->setLayout( "SEARCH", searchMap );
	}

	bool isEmpty = pool->layout( "SEARCH" ).isEmpty();
	if ( ! ( wasEmpty && isEmpty ) ) {
		mView->refreshScreen();
	}
}
