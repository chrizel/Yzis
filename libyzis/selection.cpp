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

#include "cursor.h"
#include "selection.h"
#include "debug.h"

YZSelectionPool::YZSelectionPool( YZView * view ) {
	parentView = view;
	size = 0;
	selectionPool = ( YZSelection * )calloc( size, sizeof( YZSelection ) );
}

YZSelectionPool::~YZSelectionPool( ) {
	clear( );
	free( selectionPool );
}

void YZSelectionPool::addSelection( unsigned int from_x, unsigned int from_y, unsigned int to_x, unsigned int to_y ) {
	YZCursor * from = new YZCursor( parentView );
	from->setX( from_x );
	from->setY( from_y );
	YZCursor * to = new YZCursor( parentView );
	to->setX( to_x );
	to->setY( to_y );
	addSelection( from, to );
}
void YZSelectionPool::addSelection( YZCursor * from, YZCursor * to ) {
	bool isFromSel = false;
	bool isToSel = false;
	unsigned int fromSel = locatePosition( from, &isFromSel );
	unsigned int toSel = locatePosition( to, &isToSel );
	yzDebug() << "addSelection : from: " << *from << "pos=" << fromSel << "," << isFromSel
		<< " - to: " << *to << "pos=" << toSel << "," << isToSel << endl;

	if ( isFromSel && isToSel ) {
		if ( fromSel != toSel ) {
			YZCursor * new_to = new YZCursor( selectionPool[ toSel ].to );
			// delete all selections included in new one
			removeSelection( fromSel + 1, toSel - fromSel );
			// extend fromSel selection
			selectionPool[ fromSel ].to->setCursor( new_to );
		}
	} else if ( isFromSel ) {
		removeSelection( fromSel + 1, toSel - fromSel - 1 );
		selectionPool[ fromSel ].to->setCursor( to );
	} else if ( isToSel || fromSel != toSel ) {
		YZCursor * new_to = new YZCursor( selectionPool[ toSel ].to );
		removeSelection( fromSel + 1, toSel - fromSel );
		selectionPool[ fromSel ].from->setCursor( from );
		selectionPool[ fromSel ].to->setCursor( new_to );
	} else {
		insertSelection( fromSel, from, to );
	}
}

void YZSelectionPool::delSelection( unsigned int from_x, unsigned int from_y, unsigned int to_x, unsigned int to_y ) {
	YZCursor * from = new YZCursor( parentView );
	from->setX( from_x );
	from->setY( from_y );
	YZCursor * to = new YZCursor( parentView );
	to->setX( to_x );
	to->setY( to_y );
	delSelection( from, to );
}

void YZSelectionPool::delSelection( YZCursor * from, YZCursor * to ) {
	bool isFromSel = false;
	bool isToSel = false;
	unsigned int fromSel = locatePosition( from, &isFromSel );
	unsigned int toSel = locatePosition( to, &isToSel );
	yzDebug() << "delSelection : from: " << *from << "pos=" << fromSel << "," << isFromSel
		<< " - to: " << *to << "pos=" << toSel << "," << isToSel << endl;

	YZCursor * new_from = new YZCursor( from );
	if ( new_from->getX() > 0 ) new_from->setX( new_from->getX() - 1 );
	YZCursor * new_to = new YZCursor( to );
	new_to->setX( new_to->getX() + 1 );

	bool removeFrom = fromSel < size && *selectionPool[ fromSel].from < *new_from;
	bool removeTo = toSel  < size && *selectionPool[ toSel].to < *new_to;
	yzDebug() << "removeFrom=" << removeFrom << ";removeTo=" << removeTo << endl;

	if ( ! isFromSel && ! isToSel ) {
		if ( fromSel != toSel ) {
			removeSelection( fromSel, toSel - fromSel );
		}
	} else if ( ! isToSel ) {
		if ( ! removeFrom ) selectionPool[ fromSel ].to->setCursor( new_from );
		removeSelection( fromSel + ( removeFrom ? 0 : 1 ), toSel - fromSel - ( removeFrom ? 0 : 1 ) );
	} else if ( ! isFromSel ) {
		if ( ! removeTo ) selectionPool[ toSel ].from->setCursor( new_to );
		removeSelection( fromSel, toSel - fromSel + ( removeTo ? 1 : 0 ) );
	} else if ( *selectionPool[ fromSel ].from == *from && *selectionPool[ toSel ].to == *to ) {
		removeSelection( fromSel, fromSel - toSel + 1 );
	} else if ( fromSel != toSel ) {
		if ( ! removeFrom ) selectionPool[ fromSel ].to->setCursor( new_from );
		if ( ! removeTo ) selectionPool[ toSel ].from->setCursor( new_to );
		removeSelection ( fromSel + ( removeFrom ? 0 : 1 ), toSel - fromSel - 1 + ( removeFrom ? 1 : 0 ) + ( removeTo ? 1 : 0 ) );
	} else { // split selection
		if ( removeTo && removeFrom ) removeSelection( fromSel, 1 );
		else {
			if ( ! removeTo ) insertSelection( fromSel + 1, new_to, selectionPool[ fromSel ].to );
			if ( ! removeFrom ) selectionPool[ fromSel ].to->setCursor( new_from );
		}
	}
}

void YZSelectionPool::clear( ) {
	free( selectionPool );
	size = 0;
	selectionPool = ( YZSelection * )calloc( size, sizeof( YZSelection ) );
}

bool YZSelectionPool::isSelected( YZCursor * pos ) {
	bool ret = false;
	locatePosition( pos, &ret );
	return ret;
}

void YZSelectionPool::insertSelection( unsigned int pos, YZCursor * from, YZCursor * to ) {
	++size;
	selectionPool = ( YZSelection * )realloc( selectionPool, size * sizeof( YZSelection ) );
	for ( unsigned int i = size - 1; i > pos; i-- ) {
		selectionPool[ i ] = selectionPool[ i - 1 ];
	}
	selectionPool[ pos ].from = new YZCursor( from );
	selectionPool[ pos ].to = new YZCursor( to );
}

void YZSelectionPool::removeSelection( unsigned int begin, unsigned int len ) {
	if ( len == 0 ) return;
	unsigned int i;
	for ( i = begin; i < size - len; i++ ) {
		selectionPool[ i ] = selectionPool[ i + len ];
	}
	size -= len;
	selectionPool = ( YZSelection * )realloc( selectionPool, size * sizeof( YZSelection ) );
}

int YZSelectionPool::locatePosition( YZCursor * pos, bool * isSelected ) {
	unsigned int i;
	unsigned int pos_x = pos->getX();
	unsigned int pos_y = pos->getY();
	*isSelected = false;

	for ( i = 0; ! *isSelected && i < size; i++ ) {
		if ( *pos < *selectionPool[ i ].from ) break;
		if ( *pos > *selectionPool[ i ].to ) continue;
		*isSelected = true;
	}
	if ( *isSelected ) --i;

	return i;
}

void YZSelectionPool::test( ) {
	yzDebug() << endl;
	yzDebug() << "..:: Testing YZSelectionPool ::.." << endl;

	YZCursor * c1 = new YZCursor( parentView );
	YZCursor * c2 = new YZCursor( parentView );
	c1->setX( 3 );
	c2->setX( 8 );
	addSelection( c1, c2 ); // 3,0 -> 8,0
	print();
	c2->setX( 9 );
	addSelection( c1, c2 ); // 3,0 -> 9,0
	print();
	c2->setY( 1 );
	addSelection( c1, c2 ); // 3,0 -> 9,1
	print();
	c1->setY( 3 );
	c2->setY( 3 );
	addSelection( c1, c2 ); // 3,3 -> 9,3
	print();
	c1->setX( 1 );
	addSelection( c1, c2 ); // 1,3 -> 9,3
	print();
	c1->setX( 7 );
	c1->setY( 5 );
	c2->setX( 9 );
	c2->setY( 5 );
	addSelection( c1, c2 ); // 7,5 -> 9,5
	print();

	c1->setX( 4 );
	c1->setY( 0 );
	c2->setX( 6 );
	c2->setY( 0 );
	delSelection( c1, c2 ); // 4,0 -> 6,0
	print();
	c1->setX( 5 );
	c2->setX( 10 );
	delSelection( c1, c2 ); // 5,0 -> 10,0
	print();
	c2->setY( 2 );
	delSelection( c1, c2 ); // 5,0 -> 10,2
	print();

	clear();
	yzDebug() << endl;
}

void YZSelectionPool::print( ) {
	yzDebug() << "selectionPool size = " << size << endl;
	for ( unsigned int i = 0; i < size; i++ ) {
		printSelection( i );
	}
	yzDebug() << "==========" << endl;
}

void YZSelectionPool::printSelection( unsigned int pos ) {
	yzDebug() << "selection " << pos << " : from " << selectionPool[ pos ].from->getX() << "," << selectionPool[ pos ].from->getY() \
		<< " to " << selectionPool[ pos ].to->getX() << "," << selectionPool[ pos ].to->getY() << endl;
}

