/*  This file is part of the Yzis libraries
*  Copyright (C) 2008 Loic Pauleve <panard@inzenet.org>
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

#include "drawcell.h"
#include "debug.h"

#define dbg()    yzDebug("YDrawCell")
#define err()    yzError("YDrawCell")

/************************
 * YDrawCell
 ************************/

YDrawCell::YDrawCell() :
	mSelections(0),
	mColorForeground(),
	mColorBackground(),
	mFont(),
	mContent(),
	mSteps(),
	mStepsShift(0)
{
}
YDrawCell::YDrawCell( const YDrawCell& cell ) :
	mSelections(cell.mSelections),
	mColorForeground(cell.mColorForeground),
	mColorBackground(cell.mColorBackground),
	mFont(cell.mFont),
	mContent(cell.mContent),
	mSteps(cell.mSteps),
	mStepsShift(cell.mStepsShift)
{
}
YDrawCell::~YDrawCell()
{
}

void YDrawCell::addSelection( yzis::SelectionType selType )
{
	mSelections |= selType;
}
void YDrawCell::delSelection( yzis::SelectionType selType )
{
	if ( hasSelection( selType ) )
		mSelections -= selType;
}
void YDrawCell::setForegroundColor( const YColor& color )
{
	mColorForeground = color;
}
void YDrawCell::setBackgroundColor( const YColor& color )
{
	mColorBackground = color;
}
void YDrawCell::setFont( const YFont& font )
{
	mFont = font;
}
int YDrawCell::step( const QString& data )
{
	mContent += data;
	mSteps.append(data.length());
	return data.length();
}
void YDrawCell::clear()
{
	mContent.clear();
	mSteps.clear();
	mColorBackground = YColor();
	mColorForeground = YColor();
	mSelections = 0;
	mFont = YFont();
}

int YDrawCell::widthForLength( int length ) const
{
	YASSERT(length >= 0);
	int w = 0;
	length = qMin(length, mSteps.count());
	while ( length-- ) {
		w += mSteps.at(length);
	}
	return w;
}

int YDrawCell::lengthForWidth( int width ) const
{
	YASSERT(width >= 0);
	int w = mStepsShift;
	int l = 0;
	for( ; w < width; ++l ) {
		w += mSteps[l];
	}
	return l;
}

YDrawCell YDrawCell::left_steps( int steps ) const
{
	YDrawCell c(*this);
	if ( steps < length() ) {
		c.mSteps.clear();
		int w = mStepsShift;
		for ( int i = 0; i < steps; i++ ) {
			c.mSteps << mSteps[i];
			w += mSteps[i];
		}
		c.mContent = mContent.left(w);
	}
	return c;
}
YDrawCell YDrawCell::mid_steps( int steps ) const
{
	YDrawCell c(*this);
	if ( steps > 0 ) {
		c.mStepsShift = 0;
		int w = mStepsShift;
		for ( int i = 0; i < steps; i++ ) {
			w += mSteps[i];
		}
		c.mContent = mContent.mid(w);
		c.mSteps = mSteps.mid(steps);
	}
	return c;
}

YDrawCell YDrawCell::left( int column ) const
{
	YDrawCell c(*this);
	if ( column < width() ) {
		c.mContent = mContent.left(column);
		c.mSteps.clear();
		int w = 0;
		int r = column;
		foreach( int s, mSteps ) {
			if ( r == 0 ) break;
			if ( s > r ) {
				c.mSteps << r;
				r = 0;
			} else {
				c.mSteps << s;
				w += s;
				r -= s;
			}
		}
	}
	return c;
}
YDrawCell YDrawCell::mid( int column ) const
{
	YDrawCell c(*this);
	if ( column > 0 ) {
		c.mContent = mContent.mid(column);
		c.mSteps.clear();
		int r = column;
		foreach( int s, mSteps ) {
			if ( r == 0 ) {
				c.mSteps << s;
			} else {
				if ( s > r ) {
					c.mStepsShift = s - r;
					r = 0;
				} else {
					r -= s;
				}
			}
		}
	}
	return c;
}



