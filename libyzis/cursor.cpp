/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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

YZCursor::YZCursor(YZView *vp) {
	parentView=vp;
	x_pos=0;
	y_pos=0;
}

YZCursor::YZCursor(YZView *vp, unsigned int X, unsigned int Y ) {
	parentView=vp;
	x_pos=X;
	y_pos=Y;
}

YZCursor::YZCursor( const YZCursor& c) {
	setCursor( c );
}
YZCursor::YZCursor(YZCursor *c) {
	setCursor( c );
}

YZCursor::~YZCursor() {
}

void YZCursor::setCursor( const YZCursor& c ) {
	parentView = c.parentView;
	x_pos = c.getX();
	y_pos = c.getY();
}
void YZCursor::setCursor( YZCursor *c ) {
	parentView = c->parentView;
	x_pos = c->getX();
	y_pos = c->getY();
}

bool YZCursor::operator<= ( const YZCursor & right ) const {
	return ( y_pos < right.y_pos || y_pos == right.y_pos && x_pos <= right.x_pos );
}
bool YZCursor::operator>= ( const YZCursor & right ) const {
	return ( y_pos > right.y_pos || y_pos == right.y_pos && x_pos >= right.x_pos );
}
bool YZCursor::operator< ( const YZCursor & right ) const {
	return ( y_pos < right.y_pos || y_pos == right.y_pos && x_pos < right.x_pos );
}
bool YZCursor::operator> ( const YZCursor & right ) const {
	return ( y_pos > right.y_pos || y_pos == right.y_pos && x_pos > right.x_pos );
}
bool YZCursor::operator== ( const YZCursor & right ) const {
	return ( x_pos == right.x_pos && y_pos == right.y_pos );
}
YZDebugStream &operator<< ( YZDebugStream &out, const YZCursor & c ) {
	out << "(" << c.x_pos << "," << c.y_pos << ")";
	return out;
}

