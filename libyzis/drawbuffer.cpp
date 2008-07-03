/*  This file is part of the Yzis libraries
*  Copyright (C) 2006-2008 Loic Pauleve <panard@inzenet.org>
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

#include "drawbuffer.h"
#include "debug.h"
#include "viewcursor.h"

#define dbg()    yzDebug("YDrawBuffer")
#define err()    yzError("YDrawBuffer")

/************************
 * YDrawLine
 ************************/

YDrawLine::YDrawLine() :
	mCells(),
	mSteps(),
	mBeginViewCursor(),
	mEndViewCursor()
{
	clear();
}
YDrawLine::~YDrawLine()
{
}

void YDrawLine::clear() {
	mCell = NULL;
	mCells.clear();
	mSteps.clear();
	mBeginViewCursor.setScreenX(0);
	mBeginViewCursor.setBufferX(0);
	changed = true;
}

YViewCursor YDrawLine::beginViewCursor() const {
	return mBeginViewCursor;
}
YViewCursor YDrawLine::endViewCursor() const {
	return mEndViewCursor;
}

void YDrawLine::setFont( const YFont& f )
{
    /* TODO if ( f != mCur.font ) { */ 
    mCur.font = f;
    /*  changed = true;
     } */
}
bool YDrawLine::updateColor( YColor* dest, const YColor& c )
{
    bool changed = false;
    bool was_valid = dest->isValid();
    bool is_valid = c.isValid();
    if ( was_valid != is_valid || is_valid && c.rgb() != dest->rgb() ) {
        changed = true;
        if ( is_valid ) {
            dest->setRgb( c.rgb() );
        } else {
            dest->invalidate();
        }
    }
    return changed;
}

void YDrawLine::setColor( const YColor& c )
{
    changed = updateColor(&mCur.fg, c);
}
void YDrawLine::setBackgroundColor( const YColor& c )
{
    changed = updateColor(&mCur.bg, c);
}
void YDrawLine::setSelection( int sel )
{
    if ( sel != mCur.sel ) {
        mCur.sel = sel;
        changed = true;
    }
}
int YDrawLine::push( const QString& c )
{
	if ( changed ) {
		/* set the new properties */
		insertCell();
		changed = false;
	}
	mCell->c.append( c );
	mSteps.append(c.length());
	return c.length();
}
void YDrawLine::flush() {
	mEndViewCursor = mBeginViewCursor;
	mEndViewCursor.setBufferX(mBeginViewCursor.bufferX() + mSteps.count() - 1);
	int acc;
	foreach( int s, mSteps ) {
		acc += s;
	}
	mEndViewCursor.setScreenX(mBeginViewCursor.screenX() + acc - 1);
}

void YDrawLine::insertCell( int pos )
{
	YASSERT(pos <= mCells.count());
    if ( pos == -1 ) {
        pos = mCells.count();
    }

    /* copy properties */
    YDrawCell n;
    updateColor(&n.fg, mCur.fg);
    updateColor(&n.bg, mCur.bg);
    n.sel = mCur.sel;

    if ( pos == mCells.count() ) {
		mCells << n;
    } else {
        if ( !mCells[pos].c.isEmpty() ) {
            mCells.insert(pos, n);
        } else {
            mCells.replace(pos, n);
        }
    }

    mCell =& mCells[pos];
}

void YDrawLine::setLineCursor( int bufferY, int screenY ) {
	mBeginViewCursor.setBufferY(bufferY);
	mBeginViewCursor.setScreenY(screenY);
	mEndViewCursor.setBufferY(bufferY);
	mEndViewCursor.setScreenY(screenY);
}

YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl )
{
	for ( int i = 0; i < dl.mCells.size(); ++i ) {
		out << "'" << dl.mCells[i].c << "' ";
	}
    return out;
}

YDrawSection YDrawLine::arrange( int columns ) const
{
	YDrawSection ds;

	QList<YDrawCell>::const_iterator cit = mCells.constBegin();
	QList<int>::const_iterator sit = mSteps.constBegin();

	QList<YDrawCell> cur_c;
	QList<int> cur_s;
	int cur_width = 0;

	YDrawCell c;
	int w;

	for ( ; cit != mCells.constEnd(); ++cit ) {
		c = *cit;
		w = c.c.length();
		if ( cur_width + w <= columns ) {
			cur_c << c;
			cur_width += w;
			for ( int sw = 0; (sw + *sit) <= w; ++sit ) {
				cur_s << *sit;
				sw += *sit;
			}
		} else {
			int r = columns - cur_width;
			QString left = c.c.left(r);
			QString right = c.c.mid(r);
			if ( !left.isEmpty() ) {
				c.c = left;
				w = c.c.length();
				cur_c << c;
				for ( int sw = 0; (sw + *sit) <= w; ++sit ) {
					cur_s << *sit;
					sw += *sit;
				}
			}

			YDrawLine dl;
			dl.mCells = cur_c;
			dl.mSteps = cur_s;
			dl.flush();
			ds << dl;

			cur_c.clear();
			cur_s.clear();
			cur_width = 0;

			c.c = right;
			w = c.c.length();
			cur_c << c;
			cur_width += w;
			for ( int sw = 0; sit!=mSteps.constEnd() && (sw + *sit) <= w; ++sit ) {
				cur_s << *sit;
				sw += *sit;
			}
		}
	}
	if ( cur_c.count() ) {
		YDrawLine dl;
		dl.mCells = cur_c;
		dl.mSteps = cur_s;
		dl.flush();
		ds << dl;
	}
	return ds;
}



/************************
 * YDrawBuffer
 ************************/

YDrawBuffer::YDrawBuffer() :
	mContent()
{
}
YDrawBuffer::~YDrawBuffer()
{
}

YCursor YDrawBuffer::bufferBegin() const {
	return mContent.first().first().beginViewCursor().buffer();
}
YCursor YDrawBuffer::bufferEnd() const {
	return mContent.last().last().endViewCursor().buffer();
}

void YDrawBuffer::setBufferDrawSection( int lid, YDrawSection ds )
{
	YASSERT(lid <= mContent.count());
	/* compute screenY */
	int dy = 0;
	for ( int i = 0; i < lid; ++i ) {
		dy += mContent[i].count();
	}
	/* section size changed? */
	int shift = ds.count() - mContent[lid].count();
	/* apply section */
	if ( lid == mContent.count() ) {
		mContent << ds;
	} else {
		mContent.replace(lid, ds);
	}
	for ( int j = 0; j < ds.count(); ++j ) {
		mContent[lid][j].setLineCursor(lid, dy++);
	}
	/* apply shift */
	if ( shift ) {
		for ( int i = lid + 1; i < mContent.count(); ++i ) {
			for ( int j = 0; j < mContent[i].count(); ++j ) {
				mContent[i][j].setLineCursor(lid, dy++);
			}
		}
	}
}

YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff )
{
	int dy = 0;
    for ( int i = 0; i < buff.mContent.size(); ++i ) {
		for ( int j = 0; j < buff.mContent[i].count(); ++j ) {
			out << dy++ << ": " << buff.mContent[i][j] << endl;
		}
    }
    return out;
}

