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

#include <cstdlib>

#include "selection.h"
#include "debug.h"

/*
 * YZSelectionPool
 **/

YZSelectionPool::YZSelectionPool( YZView * view ) {
	parentView = view;
	/*
	 *	selection Layouts :
	 *  DRAW : this is a fake layout, will be used to improve repaintEvent
	 *  SEARCH : selections related to search
	 *  VIEW : View mode selection
	 *  USER : user selections ( using mouse by example )
	**/
	selectionPool[ "DRAW" ].clear();
	selectionPool[ "SEARCH" ].clear();
	selectionPool[ "VIEW" ].clear();
	selectionPool[ "USER" ].clear();
}

YZSelectionPool::~YZSelectionPool( ) {
	clear( );
}

void YZSelectionPool::addSelection( const QString& layout, unsigned int from_x, unsigned int from_y, unsigned int to_x, unsigned int to_y ) {
	YZCursor from( parentView, from_x, from_y );
	YZCursor to( parentView, to_x, to_y );
	addSelection( layout, from, to, from, to );
}
void YZSelectionPool::addSelection( const QString& layout, const YZCursor& from, const YZCursor& to, const YZCursor& drawFrom, const YZCursor& drawTo ) {
	bool isFromSel = false;
	bool isToSel = false;
	unsigned int fromSel = locatePosition( layout, from, &isFromSel );
	unsigned int toSel = locatePosition( layout, to, &isToSel );
//	yzDebug() << "addSelection(" << layout << ") : from: " << from << "pos=" << fromSel << "," << isFromSel
//		<< " - to: " << to << "pos=" << toSel << "," << isToSel << endl;

	if ( isFromSel && isToSel ) {
		if ( fromSel != toSel ) {
			YZCursor new_to = ( selectionPool[ layout ][ toSel ].to() );
			// delete all selections included in new one
			removeSelection( layout, fromSel + 1, toSel - fromSel );
			// extend fromSel selection
			selectionPool[ layout ][ fromSel ].setTo( new_to );
		}
	} else if ( isFromSel ) {
		removeSelection( layout, fromSel + 1, toSel - fromSel - 1 );
		selectionPool[ layout ][ fromSel ].setTo( to );
	} else if ( isToSel || fromSel != toSel ) {
		YZCursor new_to( selectionPool[ layout ].find( toSel ).data().to() );
		removeSelection( layout, fromSel + 1, toSel - fromSel );
		selectionPool[ layout ][ fromSel ].setFrom( from );
		selectionPool[ layout ][ fromSel ].setTo( new_to );
	} else {
		insertSelection( layout, fromSel, from, to, drawFrom, drawTo );
	}
}

void YZSelectionPool::delSelection( const QString& layout, unsigned int from_x, unsigned int from_y, unsigned int to_x, unsigned int to_y ) {
	YZCursor from( parentView, from_x, from_y );
	YZCursor to( parentView, to_x, to_y );
	delSelection( layout, from, to, from, to );
}

void YZSelectionPool::delSelection( const QString& layout, const YZCursor& from, const YZCursor& to, const YZCursor& drawFrom, const YZCursor& /*drawTo*/ ) {
	bool isFromSel = false;
	bool isToSel = false;
	unsigned int fromSel = locatePosition( layout, from, &isFromSel );
	unsigned int toSel = locatePosition( layout, to, &isToSel );
//	yzDebug() << "delSelection : from: " << from << "pos=" << fromSel << "," << isFromSel
//		<< " - to: " << to << "pos=" << toSel << "," << isToSel << endl;

	YZCursor new_from( from );
//	if ( new_from.getX() > 0 ) new_from.setX( new_from.getX() - 1 );
	YZCursor new_to( to );
//	new_to.setX( new_to.getX() + 1 );

	bool removeFrom = selectionPool[ layout ].contains( fromSel ) && selectionPool[ layout ][ fromSel ].from() >= new_from;
	bool removeTo = selectionPool[ layout ].contains( toSel ) && selectionPool[ layout ][ toSel ].to() <= new_to;
//	yzDebug() << "removeFrom=" << removeFrom << ";removeTo=" << removeTo << endl;

	if ( ! isFromSel && ! isToSel ) {
		if ( fromSel != toSel ) {
			removeSelection( layout, fromSel, toSel - fromSel );
		}
	} else if ( ! isToSel ) {
		if ( ! removeFrom ) selectionPool[ layout ][ fromSel ].setTo( new_from );
		removeSelection( layout, fromSel + ( removeFrom ? 0 : 1 ), toSel - fromSel - ( removeFrom ? 0 : 1 ) );
	} else if ( ! isFromSel ) {
		if ( ! removeTo ) selectionPool[ layout ][ toSel ].setFrom( new_to );
		removeSelection( layout, fromSel, toSel - fromSel + ( removeTo ? 1 : 0 ) );
	} else if ( selectionPool[ layout ][ fromSel ].from() == from && selectionPool[ layout ][ toSel ].to() == to ) {
		removeSelection( layout, fromSel, fromSel - toSel + 1 );
	} else if ( fromSel != toSel ) {
		if ( ! removeFrom ) selectionPool[ layout ][ fromSel ].setTo( new_from );
		if ( ! removeTo ) selectionPool[ layout ][ toSel ].setFrom( new_to );
		removeSelection ( layout, fromSel + ( removeFrom ? 0 : 1 ), toSel - fromSel - 1 + ( removeFrom ? 1 : 0 ) + ( removeTo ? 1 : 0 ) );
	} else { // split selection, fromSel == toSel
		if ( removeTo && removeFrom ) removeSelection( layout, fromSel, 1 );
		else if ( removeFrom ) selectionPool[ layout ][ fromSel ].setFrom( new_to );
		else if ( removeTo ) selectionPool[ layout ][ fromSel ].setTo( new_from );
		else {
			selectionPool[ layout ][ fromSel ].setTo( new_from );
			insertSelection( layout, fromSel + 1, new_to, selectionPool[ layout ][ fromSel ].to(), drawFrom, selectionPool[ layout ][ fromSel ].drawTo() );
		}
	}
}

void YZSelectionPool::clear( ) {
	YZSelectionLayout::Iterator it = selectionPool.begin(), end = selectionPool.end();
	for ( ; it != end; ++it )
		clear( it.key() );
}

void YZSelectionPool::clear( const QString& layout ) {
	selectionPool[ layout ].clear( );
}

YZSelectionMap  YZSelectionPool::layout( const QString& layout ) {
	return selectionPool[ layout ];
}

bool YZSelectionPool::isSelected( const YZCursor& pos ) {
	bool ret = false;
	YZSelectionLayout::Iterator it = selectionPool.begin(), end = selectionPool.end();
	for ( ; ! ret && it != end; ++it )
		if ( it.key() != "DRAW" )  // draw is not a selection
			locatePosition( it.key(), pos, &ret );
	return ret;
}

void YZSelectionPool::insertSelection( const QString& layout, unsigned int pos, const YZCursor& from, const YZCursor& to, const YZCursor& drawFrom, const YZCursor& drawTo ) {
	unsigned int size = selectionPool[ layout ].size( ) + 1;
	for ( unsigned int i = size - 1; i > pos; i-- ) {
		selectionPool[ layout ][ i ] = selectionPool[ layout ][ i - 1 ];
	}
	YZSelection new_sel( from, to, drawFrom, drawTo );
		/*
	new_sel.from = new YZCursor( from );
	new_sel.to = new YZCursor( to );
	new_sel.drawFrom = new YZCursor( drawFrom );
	new_sel.drawTo = new YZCursor( drawTo );*/
	selectionPool[ layout ].insert( pos, new_sel );
}

void YZSelectionPool::removeSelection( const QString& layout, unsigned int begin, unsigned int len ) {
	if ( len == 0 ) return;
	unsigned int i;
	unsigned int size = selectionPool[ layout ].size( );
	for ( i = begin; i < size - len; ++i ) {
		selectionPool[ layout ][ i ] = selectionPool[ layout ][ i + len ];
	}
	for ( ; i < size; i++ ) {
/*		delete selectionPool[ layout ][ i ].from;
		delete selectionPool[ layout ][ i ].to;
		delete selectionPool[ layout ][ i ].drawFrom;
		delete selectionPool[ layout ][ i ].drawTo;*/
		selectionPool[ layout ].remove( i );
	}
}

void YZSelectionPool::debug( const QString& layout ) {
	unsigned int size = selectionPool[ layout ].size( );
	for ( unsigned int i = 0; i < size; i++ ) {
		yzDebug() << "[" << layout << "] (" << i << ") from=" << selectionPool[ layout ][ i ].from() << "; to=" << selectionPool[ layout ][ i ].to() << endl;
	}
}

int YZSelectionPool::locatePosition( const QString& layout, const YZCursor& pos, bool * isSelected ) {
	unsigned int i;
	*isSelected = false;
	unsigned int size = selectionPool[ layout ].size( );
	for ( i = 0; ! *isSelected && i < size; i++ ) {
		if ( pos < selectionPool[ layout ][ i ].from() ) break;
		if ( pos > selectionPool[ layout ][ i ].to() ) continue;
		*isSelected = true;
	}
	if ( *isSelected ) --i;

	return i;
}

