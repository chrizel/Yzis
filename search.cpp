/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>,
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

#include <QList>

#include "portability.h"
#include "debug.h"
#include "action.h"
#include "search.h"
#include "view.h"
#include "session.h"
#include "buffer.h"
#include "selection.h"

#define dbg()    yzDebug("YZSearch")
#define err()    yzError("YZSearch")

struct YZSearch::Private
{		
	void setCurrentSearch( const QString& pattern );
	YZCursor doSearch( YZBuffer *buffer, const YZCursor from, const QString& pattern, bool reverse, bool skipline, bool* found );
	void highlightSearch( YZView *view, YZSelectionMap searchMap );
	bool active();
	
	QString mCurrentSearch;
};

YZSearch::YZSearch() 
	: d(new Private)
{
	d->mCurrentSearch = QString();
}

YZSearch::~YZSearch() 
{
	delete d;
}

YZCursor YZSearch::forward( YZBuffer *buffer, const QString& pattern, bool* found, const YZCursor from ) {
	YZCursor tmp = d->doSearch( buffer, from, pattern, false, false, found );
	YZSession::self()->saveJumpPosition( tmp );
	
	return tmp;
}

YZCursor YZSearch::backward( YZBuffer *buffer, const QString& pattern, bool* found, const YZCursor from ) {
	YZCursor tmp = d->doSearch( buffer, from, pattern, true, false, found );
	YZSession::self()->saveJumpPosition( tmp );
	
	return tmp;
}

YZCursor YZSearch::replayForward( YZBuffer *buffer, bool* found, const YZCursor from, bool skipline /*=false*/) {
	return d->doSearch( buffer, from, d->mCurrentSearch, false, skipline, found );
}
YZCursor YZSearch::replayBackward( YZBuffer *buffer, bool* found, const YZCursor from, bool skipline /*=false*/) {
	return d->doSearch( buffer, from, d->mCurrentSearch, true, skipline, found );
}

const QString& YZSearch::currentSearch() const {
	return d->mCurrentSearch;
}

bool YZSearch::active()
{
	return d->active();
}

bool YZSearch::Private::active() {
	return ! ( mCurrentSearch.isNull() || mCurrentSearch.isEmpty() );
}

YZCursor YZSearch::Private::doSearch( YZBuffer *buffer, const YZCursor from, const QString& pattern, bool reverse, bool skipline, bool* found ) {
	dbg() << "YZSearch::doSearch " << pattern << ", " << reverse << ", " << endl;
	*found = false;
	setCurrentSearch( pattern );
	int direction = reverse ? 0 : 1;

	YZCursor cur( from ); // there used to be a null check here, but passing in a null cursor doesn't make sense

	if ( ! active() ) return cur;

	if ( skipline ) {
		cur.setX( 0 );
		if ( ! reverse ) cur.setY( qMin( (int)(cur.y() + 1), (int)(buffer->lineCount() - 1) ) );
	} else {
		cur.setX( qMax( (int)(cur.x() + direction), 0 ) );
	}

	// define absolute ranges for the buffer
	YZCursor top( 0, 0 );
	YZCursor bottom;
	bottom.setY( buffer->lineCount() - 1 );
	bottom.setX( qMax( (int)(buffer->textline( bottom.y() ).length() - 1), 0 ) );

	// cursor for the end of the search range
	YZCursor end( bottom );
	if ( reverse ) end = top;

	// use an action to do the search
	int matchedLength;
//	dbg() << "begin = " << cur << endl;
	YZCursor ret = buffer->action()->search( buffer, pattern, cur, end, &matchedLength, found );

	// check to see if we need to wrap
	if ( ! *found ) {
		dbg() << "search hits top or bottom" << endl;
		
		// adjust the cursors for the next search
		end = cur;
		if ( reverse ) {
			cur = bottom;
		} else {
			cur = top;
		}
//		dbg() << "begin = " << cur << ", end = " << end << endl;
	
		// repeat the search with the new bounds
		ret = buffer->action()->search( buffer, pattern, cur, end, &matchedLength, found );
		if ( *found ) {
			YZView *view = YZSession::self()->findViewByBuffer( buffer );
			if ( view && reverse ) {
				view->guiDisplayInfo( _("search hit TOP, continuing at BOTTOM") );
			} else if ( view ) {
				view->guiDisplayInfo( _("search hit BOTTOM, continuing at TOP") );
			}
		}
	}
//	dbg() << "ret = " << ret << endl;

	return ret;
}

void YZSearch::Private::setCurrentSearch( const QString& pattern ) {
	if ( mCurrentSearch == pattern ) return;
	mCurrentSearch = pattern;

	YZSelectionMap searchMap;
	foreach( YZBuffer *b, YZSession::self()->buffers() ) {
		QList<YZView*> views = b->views();

		searchMap.clear();

		/** search all **/
		bool doIt = YZSession::self()->getBooleanOption( "hlsearch" );
		if ( doIt ) {
			YZView* v = views.front();
			YZCursor from( 0, 0 );
			YZCursor cur( from );
			YZCursor end;
			end.setY( b->lineCount() - 1 );
			end.setX( qMax( (int)(b->textline( end.y() ).length() - 1), 0 ) );

			bool found = true;
			int matchedLength = 0;
			int pos = 0;
			do {
				from = b->action()->search( v->myBuffer(), mCurrentSearch, cur, end, &matchedLength, &found );
				if ( found && matchedLength > 0 ) {
					cur = from;
					cur.setX( cur.x() + matchedLength - 1 );
					YZInterval sel( from, cur );
					cur.setX( cur.x() + 1 );
					searchMap.insert( pos++, sel );
				}
			} while ( found );
		}

		foreach( YZView *view, views )
			highlightSearch( view, searchMap );
	}
}

void YZSearch::highlightLine( YZBuffer* buffer, int line ) {
	if ( d->mCurrentSearch.isNull() || d->mCurrentSearch.isEmpty() ) return;
	bool doIt = YZSession::self()->getBooleanOption( "hlsearch" );
	if ( doIt ) {
		QList<YZView*> views = buffer->views();
		YZView* v = views.front();
		YZCursor from( 0, line );
		YZCursor cur( from );
		YZCursor end( buffer->textline( line ).length(), line );

		YZSelection* searchMap = v->getSelectionPool()->search();
		searchMap->delInterval( YZInterval( from, end ) );

		if ( end.x() > 0 ) end.setX( end.x() - 1 );

		bool found;
		int matchedLength = 0;
		do {
			from = buffer->action()->search( v->myBuffer(), d->mCurrentSearch, cur, end, &matchedLength, &found );
			if ( found && matchedLength > 0 ) {
				cur = from;
				cur.setX( cur.x() + matchedLength - 1 );
				searchMap->addInterval( YZInterval( from, cur ) );
				cur.setX( cur.x() + 1 );
//				dbg() << "cur = " << cur << "; end = " << end << endl;
			}
		} while ( found );

		foreach( YZView *view, views ) {
			view->getSelectionPool()->setSearch( searchMap );
			view->sendPaintEvent( 0, line, qMax( (int)(buffer->textline( line ).length() - 1), 0 ), line );
		}
	}
}

void YZSearch::shiftHighlight( YZBuffer* buffer, int fromLine, int shift ) {
	QList<YZView*> views = buffer->views();
	if ( views.isEmpty() ) // no views
		return;
	YZView* v = views.front();
	if ( v ) {
		YZSelectionMap searchMap = v->getSelectionPool()->search()->map();

		if ( shift+fromLine < 0 ) fromLine = -shift;
		int size = searchMap.size();
		for ( int i = 0; i < size; i++ ) {
			YZCursor to = searchMap[ i ].toPos();
			if ( to.y() < fromLine ) continue;

			YZCursor from = searchMap[ i ].fromPos();
			from.setY( from.y() + shift);
			to.setY( to.y() + shift);

			searchMap[ i ].setFromPos( from );
			searchMap[ i ].setToPos( to );
		}

		foreach( YZView *view, views ) 
			d->highlightSearch( view, searchMap );
	}
}

void YZSearch::Private::highlightSearch( YZView *view, YZSelectionMap searchMap ) {
	view->setPaintAutoCommit( false );
	YZSelection* vMap = view->getSelectionPool()->search();
	view->sendPaintEvent( vMap->map(), false );
	vMap->clear();
	if ( YZSession::self()->getBooleanOption( "hlsearch" ) ) {
		vMap->setMap( searchMap );
//		dbg() << "new search Map : " << *(vMap) << endl;
		view->sendPaintEvent( vMap->map() );
	}
	view->commitPaintEvent();
}

void YZSearch::update() {
	if ( ! active() ) return;
	if ( YZSession::self()->getBooleanOption( "hlsearch" ) ) {
		// force creating hl selection
		QString pattern = d->mCurrentSearch;
		d->mCurrentSearch = "";
		d->setCurrentSearch( pattern );
	} else {
		YZSelectionMap searchMap;
		// clear current hl search selection
		foreach( YZBuffer *b, YZSession::self()->buffers()) 
			foreach( YZView *view, b->views() ) 
				d->highlightSearch( view, searchMap );
	}
}

