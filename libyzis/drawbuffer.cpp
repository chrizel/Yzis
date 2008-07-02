/*  This file is part of the Yzis libraries
*  Copyright (C) 2006 Loic Pauleve <panard@inzenet.org>
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
#include "view.h"
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
	mBeginViewCursor.setScreenX(0);
	mBeginViewCursor.setBufferX(0);
}
YDrawLine::~YDrawLine()
{
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
    //YCursor pos( v_xi + mCell->c.length(), m_y );
    //QPoint step(1, 0);
    for ( int i = 0; i < c.length(); ++i ) {
		/* TODO : selection
        int sel = YSelectionPool::None;
        foreach( YSelectionPool::SelectionLayout layout, m_sel.keys() ) {
            if ( m_sel[layout].contains(pos) )
                sel |= layout;
        }
        setSelection( sel );
		*/
		if ( changed ) {
			/* set the new properties */
			insertCell();
		}
		mCell->c.append( c );
		mSteps.append(c.length());
        //pos = pos + step;
    }
	return i;
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
    if ( pos == -1 ) {
        pos = mCells->size();
    }

    /* copy properties */
    YDrawCell n;
    updateColor(&n.fg, mCur.fg);
    updateColor(&n.bg, mCur.bg);
    n.sel = mCur.sel;

    if ( pos >= mCells->size() ) {
        mCells->resize(pos + 1);
        mCells->replace(pos, n);
    } else {
        if ( mCells->at(pos).c.length() > 0 ) {
            mCells->insert(pos, n);
        } else {
            mCells->replace(pos, n);
        }
    }

    mCell = &(*mCells)[pos];
}

void YDrawLine::setLineCursor( int bufferY, int screenY ) {
	mBeginViewCursor.setBufferY(bufferY);
	mBeginViewCursor.setScreenY(screenY);
	mEndViewCursor.setBufferY(bufferY);
	mEndViewCursor.setScreenY(screenY);
}

YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl )
{
	for ( int i = 0; j < dl.mCells.size(); ++i ) {
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

	for ( cit != mCells.constEnd(); ++cit ) {
		c = *cit;
		w = c.c.length();
		if ( cur_width + w <= columns ) {
			cur_c << c;
			for ( int sw = 0; (sw + *sit) <= w; ++sit ) {
				cur_s << *sit;
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
				}
			}

			YDrawLine dl;
			dl.mCells = cur_c;
			dl.mSteps = cur_s;
			dl.flush();
			ds << dl;

			cur_c.clear();
			cur_s.clear();

			c.c = right;
			w = c.c.length();
			cur_c << c;
			for ( int sw = 0; (sw + *sit) <= w; ++sit ) {
				cur_s << *sit;
			}
		}
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

void YDrawBuffer::setBufferDrawSection( int lid, YDrawSection ds )
{
	if ( lid > mContent.count() ) {
		mContent.resize(lid+1);
	}
	/* compute screenY */
	int dy = 0;
	for ( int i = 0; i < lid; ++i ) {
		dy += mContent[i].count();
	}
	/* section size changed? */
	int shift = ds.count() - mContent[lid].count()
	/* apply section */
	mContent.replace(lid, ds);
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
	dy = 0;
    for ( int i = 0; i < buff.mContent.size(); ++i ) {
		for ( int j = 0; j < buff.mContent[i].count(); ++j ) {
			out << dy++ << ": " << buff.mContent[i][j] << endl;
		}
    }
    return out;
}

