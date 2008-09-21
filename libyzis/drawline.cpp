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

#include "drawline.h"
#include "drawcell.h"
#include "drawbuffer.h"

#include "debug.h"
#define dbg()    yzDebug("YDrawLine")
#define err()    yzError("YDrawLine")

/************************
 * YDrawLine
 ************************/

YDrawLine::YDrawLine() :
	QList<YDrawCell>()
{
	clear();
}
YDrawLine::~YDrawLine()
{
}

void YDrawLine::clear() {
	QList<YDrawCell>::clear();
	mCur.clear();
	mCell = NULL;
	mWidth = 0;
	changed = true;
}

void YDrawLine::setFont( const YFont& f )
{
	/* TODO */
    /* if ( f != mCur.font ) {  */
	mCur.setFont(f);
	/*
      changed = true;
     } */
}

void YDrawLine::setColor( const YColor& c )
{
	if ( mCur.foregroundColor() != c ) {
		mCur.setForegroundColor(c);
		changed = true;
	}
}
void YDrawLine::setBackgroundColor( const YColor& c )
{
	if ( mCur.backgroundColor() != c ) {
		mCur.setBackgroundColor(c);
		changed = true;
	}
}

int YDrawLine::step( const QString& c )
{
	if ( changed ) {
		append(YDrawCell(mCur));
		mCell =& (*this)[size()-1];
		changed = false;
	}
	return mCell->step(c);
}
void YDrawLine::flush()
{
	mWidth = 0;
	mLength = 0;
	for ( int i = 0; i < count(); ++i ) {
		mWidth +=  at(i).width();
		mLength += at(i).length();
	}
}

YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl )
{
	for ( int i = 0; i < dl.size(); ++i ) {
		out << "'" << dl[i].content() << "' ";
	}
    return out;
}

YDrawSection YDrawLine::arrange( int columns ) const
{
	if ( count() == 0 ) {
		return YDrawSection() << YDrawLine();
	}

	YDrawSection ds;
	QList<YDrawCell> line;
	int line_width = 0;

	foreach( YDrawCell c, *this ) {
		int w = c.width();
		if ( line_width + w <= columns ) {
			line << c;
			line_width += w;
		} else {
			/* split current cell */
			int r = columns - line_width;
			if ( r ) {
				line << c.left(r);
			}
			YDrawLine dl;
			//dl.mCells = line;
			dl.flush();
			ds << dl;
			line.clear();

			YDrawCell right = c.right(r);
			line << right;
			line_width = right.width();
		}
	}
	if ( line.count() > 0 ) {
		YDrawLine dl;
		//dl.mCells = line;
		dl.flush();
		ds << dl;
	}
	return ds;
}

