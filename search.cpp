/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>,
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

YZCursor YZSearch::forward( YZView* mView, const QString& pattern, bool* found, YZCursor* from ) {
	return doSearch( mView, from, pattern, false, false, found );
}
YZCursor YZSearch::backward( YZView* mView, const QString& pattern, bool* found, YZCursor* from ) {
	return doSearch( mView, from, pattern, true, false, found );
}
YZCursor YZSearch::replayForward( YZView* mView, bool* found, YZCursor* from, bool skipline ) {
	return doSearch( mView, from, mCurrentSearch, false, skipline, found );
}
YZCursor YZSearch::replayBackward( YZView* mView, bool* found, YZCursor* from, bool skipline ) {
	return doSearch( mView, from, mCurrentSearch, true, skipline, found );
}

const QString& YZSearch::currentSearch() const {
	return mCurrentSearch;
}

bool YZSearch::active() {
	return ! ( mCurrentSearch.isNull() || mCurrentSearch.isEmpty() );
}

YZCursor YZSearch::doSearch( YZView* mView, YZCursor* from, const QString& pattern, bool reverse, bool skipline, bool* found ) {
	yzDebug() << "YZSearch::doSearch " << pattern << ", " << reverse << ", " << endl;
	*found = false;
	setCurrentSearch( pattern );
	int direction = reverse ? 0 : 1;

	YZCursor cur( mView );
	if ( from != NULL )
		cur.setCursor( from );
	else
		cur.setCursor( mView->getBufferCursor() );

	if ( ! active() ) return cur;

	if ( skipline ) {
		cur.setX( 0 );
#if QT_VERSION < 0x040000
		if ( ! reverse ) cur.setY( QMIN( (int)(cur.getY() + 1), (int)(mView->myBuffer()->lineCount() - 1) ) );
#else
		if ( ! reverse ) cur.setY( qMin( (int)(cur.getY() + 1), (int)(mView->myBuffer()->lineCount() - 1) ) );
#endif
	} else {
#if QT_VERSION < 0x040000
		cur.setX( QMAX( (int)(cur.getX() + direction), 0 ) );
#else
		cur.setX( qMax( (int)(cur.getX() + direction), 0 ) );
#endif
	}
	YZCursor top( mView, 0, 0 );
	YZCursor bottom( mView );
	bottom.setY( mView->myBuffer()->lineCount() - 1 );
#if QT_VERSION < 0x040000
	bottom.setX( QMAX( (int)(mView->myBuffer()->textline( bottom.getY() ).length() - 1), 0 ) );
#else
	bottom.setX( qMax( (int)(mView->myBuffer()->textline( bottom.getY() ).length() - 1), 0 ) );
#endif

	YZCursor end( bottom );
	if ( reverse ) end.setCursor( top );

	unsigned int matchedLength;
	yzDebug() << "begin = " << cur << endl;
	YZCursor ret = mView->myBuffer()->action()->search( mView, pattern, cur, end, reverse, &matchedLength, found );
	if ( ! *found ) {
		yzDebug() << "search hits top or bottom" << endl;
		end.setCursor( cur );
		if ( reverse ) 
			cur.setCursor( bottom );
		else 
			cur.setCursor( top );
		yzDebug() << "begin = " << cur << ", end = " << end << endl;
		ret = mView->myBuffer()->action()->search( mView, pattern, cur, end, reverse, &matchedLength, found );
		if ( *found ) {
			if ( reverse )
				mView->displayInfo( QObject::tr("search hit TOP, continuing at BOTTOM") );
			else
				mView->displayInfo( QObject::tr("search hit BOTTOM, continuing at TOP") );
		}
	}
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
#if QT_VERSION < 0x040000
		YZView* v;
		YZBuffer* b = it.data();
		QPtrList<YZView> views = b->views();
#else
		YZBuffer* b = it.value();
		QVector<YZView*> views = b->views();
#endif

		searchMap.clear();

		/** search all **/
		bool doIt = false;
#if QT_VERSION < 0x040000
		for( v = views.first(); ! doIt && v ; v = views.next() )
			doIt = doIt || v->getLocalBoolOption( "hlsearch" );
#else
		for (int i = 0 ; i < views.size(); ++i )
			doIt = doIt || views.at(i)->getLocalBoolOption("hlsearch");
#endif
		if ( doIt ) {
			YZView* v = views.first();
			YZCursor from( v, 0, 0 );
			YZCursor cur( from );
			YZCursor end( v );
			end.setY( b->lineCount() - 1 );
#if QT_VERSION < 0x040000
			end.setX( QMAX( (int)(b->textline( end.getY() ).length() - 1), 0 ) );
#else
			end.setX( qMax( (int)(b->textline( end.getY() ).length() - 1), 0 ) );
#endif

			bool found = true;
			unsigned int matchedLength = 0;
			unsigned int pos = 0;
			do {
				from = b->action()->search( v, mCurrentSearch, cur, end, false, &matchedLength, &found );
				if ( found && matchedLength > 0 ) {
					cur.setCursor( from );
					cur.setX( cur.getX() + matchedLength - 1 );
					YZSelection sel( from, cur, from, cur );
					cur.setX( cur.getX() + 1 );
					searchMap.insert( pos++, sel );
				}
			} while ( found );
		}

#if QT_VERSION < 0x040000
		for( v = views.first(); v; v = views.next() )
			highlightSearch( v, searchMap );
#else
		for (int i = 0 ; i < views.size(); ++i )
			highlightSearch( views.at(i), searchMap );
#endif
	}
}

void YZSearch::highlightLine( YZBuffer* buffer, unsigned int line ) {
	if ( mCurrentSearch.isNull() || mCurrentSearch.isEmpty() ) return;
	bool doIt = false;
#if QT_VERSION < 0x040000
	QPtrList<YZView> views = buffer->views();
	for( YZView* v = views.first(); ! doIt && v ; v = views.next() )
		doIt = doIt || v->getLocalBoolOption( "hlsearch" );
#else
	QVector<YZView*> views = buffer->views();
	for (int i = 0 ; i < views.size(); ++i )
		doIt = doIt || views.at(i)->getLocalBoolOption("hlsearch");
#endif
	if ( doIt ) {
		YZView* v = views.first();
		YZCursor from( v, 0, line );
		YZCursor cur( from );
		YZCursor end( v, buffer->textline( line ).length(), line );

		YZSelectionPool* pool = v->getSelectionPool();
		pool->delSelection( "SEARCH", from, end, from, end );

		if ( end.getX() > 0 ) end.setX( end.getX() - 1 );

		bool found;
		unsigned int matchedLength = 0;
		do {
			from = buffer->action()->search( v, mCurrentSearch, cur, end, false, &matchedLength, &found );
			if ( found && matchedLength > 0 ) {
				cur.setCursor( from );
				cur.setX( cur.getX() + matchedLength - 1 );
				pool->addSelection( "SEARCH", from, cur, from, cur );
				cur.setX( cur.getX() + 1 );
//				yzDebug() << "cur = " << cur << "; end = " << end << endl;
			}
		} while ( found );

#if QT_VERSION < 0x040000
		for( v = views.first(); v; v = views.next() ) {
			v->getSelectionPool()->setLayout( "SEARCH", pool->layout( "SEARCH" ) );
			v->sendPaintEvent( 0, line, QMAX( (int)(buffer->textline( line ).length() - 1), 0 ), line );
		}
#else
		for (int i = 0 ; i < views.size(); ++i ) {
			views.at(i)->getSelectionPool()->setLayout( "SEARCH", pool->layout( "SEARCH" ) );
			views.at(i)->sendPaintEvent( 0, line, qMax( (int)(buffer->textline( line ).length() - 1), 0 ), line );
		}
#endif
	}
}

void YZSearch::shiftHighlight( YZBuffer* buffer, unsigned int fromLine, int shift ) {
#if QT_VERSION < 0x040000
	QPtrList<YZView> views = buffer->views();
#else
	QVector<YZView*> views = buffer->views();
#endif
	YZView* v = views.first();
	if ( v ) {
		YZSelectionMap searchMap = v->getSelectionPool()->layout( "SEARCH" );

		if ( ( ( int )( shift + fromLine ) ) < 0 ) fromLine = -shift;
		unsigned int size = searchMap.size();
		for ( unsigned int i = 0; i < size; i++ ) {
			YZCursor to = searchMap[ i ].to();
			if ( to.getY() < fromLine ) continue;

			YZCursor from = searchMap[ i ].from();
			from.setY( from.getY() + shift);
			to.setY( to.getY() + shift);

			searchMap[ i ].setFrom( from );
			searchMap[ i ].setTo( to );
			searchMap[ i ].setDrawFrom( from );
			searchMap[ i ].setDrawTo( to );
		}

#if QT_VERSION < 0x040000
		for( v = views.first(); v; v = views.next() )
			highlightSearch( v, searchMap );
#else
		for (int i = 0 ; i < views.size(); ++i )
			highlightSearch( views.at(i), searchMap );
#endif
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
	if ( ! ( wasEmpty && isEmpty ) && mView->getLocalBoolOption( "hlsearch" ) ) {
		mView->sendRefreshEvent();
	}
}
