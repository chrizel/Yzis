/* This file is part of the Yzis libraries
 *  Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org> 
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

#include "cursor.h"
#include "debug.h"

YZCursor::YZCursor(YZView *vp) {
	parentView=vp;
	x_pos=0;
	y_pos=0;
}

YZCursor::YZCursor(YZCursor *c) {
	setCursor( c );
}

YZCursor::~YZCursor() {
}

void YZCursor::setCursor( YZCursor *c ) {
	parentView = c->parentView;
	x_pos = c->getX();
	y_pos = c->getY();
}

bool YZCursor::lt( YZCursor *c ) {
	return ( getY() < c->getY() || getY() == c->getY() && getX() < c->getX() );
}

bool operator<= ( YZCursor &left, const YZCursor & right ) {
	yzDebug() << "operator <=" << endl;
	return ( left.y_pos < right.y_pos || left.y_pos == right.y_pos && left.x_pos <= right.x_pos );
}
bool operator>= ( YZCursor &left, const YZCursor & right ) {
	yzDebug() << "operator >=" << endl;
	return ( left.y_pos > right.y_pos || left.y_pos == right.y_pos && left.x_pos >= right.x_pos );
}
bool operator< ( YZCursor &left, const YZCursor & right ) {
	yzDebug() << "operator < " << endl;
	return ( left.y_pos < right.y_pos || left.y_pos == right.y_pos && left.x_pos < right.x_pos );
}
bool operator> ( YZCursor &left, const YZCursor & right ) {
	yzDebug() << "operator >" << endl;
	return ( left.y_pos > right.y_pos || left.y_pos == right.y_pos && left.x_pos > right.x_pos );
}
bool operator== ( YZCursor &left, const YZCursor & right ) {
	yzDebug() << "operator ==" << endl;
	return ( left.x_pos == right.x_pos && left.y_pos == right.y_pos );
}

