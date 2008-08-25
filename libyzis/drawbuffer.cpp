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
	mSteps()
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
	changed = true;
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
    if ( was_valid != is_valid || (is_valid && c.rgb() != dest->rgb()) ) {
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
    changed |= updateColor(&mCur.fg, c);
}
void YDrawLine::setBackgroundColor( const YColor& c )
{
    changed |= updateColor(&mCur.bg, c);
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
	/* useless for now
	int acc = 0;
	foreach( int s, mSteps ) {
		acc += s;
	}
	mEndViewCursor.setScreenX(mBeginViewCursor.screenX() + acc - 1);
	*/
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

YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl )
{
	for ( int i = 0; i < dl.mCells.size(); ++i ) {
		out << "'" << dl.mCells[i].c << "' ";
	}
    return out;
}

YDrawSection YDrawLine::arrange( int columns ) const
{
	if ( mCells.count() == 0 ) {
		return YDrawSection() << YDrawLine();
	}
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
			for ( int sw = 0; sit != mSteps.constEnd() && (sw + *sit) <= w; ++sit ) {
				cur_s << *sit;
				sw += *sit;
			}
		} else {
			/* split current cell */
			int r = columns - cur_width;
			QString left = c.c.left(r);
			QString right = c.c.mid(r);
			int rs = 0;
			if ( !left.isEmpty() ) {
				c.c = left;
				w = c.c.length();
				cur_c << c;
				int sw;
				for ( sw = 0; (sw + *sit) < w; ++sit ) {
					cur_s << *sit;
					sw += *sit;
				}
				cur_s << w - sw;
				rs = *sit - (w-sw);
				++sit;
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
			if ( rs )
				cur_s << rs;
			int sw = rs;
			for ( ; sit!=mSteps.constEnd() && (sw + *sit) <= w; ++sit ) {
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

YDrawBuffer::YDrawBuffer( int columns, int lines ) :
	mContent(),
	mScreenOffset(0,0),
	mEOLCell()
{
	mTopBufferLine = 0;
	mEOLCell.c = " ";
	setScreenSize(columns, lines);
	mContent << (YDrawSection() << YDrawLine());
}
YDrawBuffer::~YDrawBuffer()
{
}

void YDrawBuffer::setScreenSize( int columns, int lines ) 
{
	mScreenWidth = columns;
	mScreenHeight = lines;
}

int YDrawBuffer::currentHeight() const
{
	int dy = 0;
	for ( int i = 0; i < mContent.count(); ++i ) {
		dy += mContent[i].count();
	}
	return dy;
}

int YDrawBuffer::setBufferDrawSection( int bl, YDrawSection ds, int* shift )
{
	int lid = bl - mTopBufferLine;
	YASSERT(lid <= mContent.count());
	/* compute screenY */
	int dy = 0;
	for ( int i = 0; i < lid; ++i ) {
		dy += mContent[i].count();
	}
	YASSERT(dy < mScreenHeight);
	int m_shift = 0;
	/* apply section */
	if ( lid == mContent.count() ) {
		mContent << ds;
	} else {
		 /* section size changed? */
		m_shift = ds.count() - mContent[lid].count();
		mContent.replace(lid, ds);
		if ( m_shift > 0 ) { 
			/* remove out of screen lines */
			int my_h = currentHeight();
			int i = mContent.size() - 1;
			while ( my_h - mContent[i].count() > mScreenHeight ) {
				my_h -= mContent[i].count();
				mContent.takeAt(i);
			}
		}
	}
	if ( shift ) *shift = m_shift;
	return dy;
}
YInterval YDrawBuffer::deleteFromBufferDrawSection( int bl )
{
	int lid = bl - mTopBufferLine;
	YASSERT(0 <= lid)
	YInterval affected;
	if ( lid >= mContent.count() ) {
		return affected;
	}
	/* compute screenY */
	int dy = bufferDrawSectionScreenLine(bl);
	affected.setFrom(YBound(YCursor(0,dy)));
	while ( lid < mContent.count() ) {
		dy += mContent[lid].count();
		mContent.takeAt(lid);
	}
	affected.setTo(YBound(YCursor(0, dy), true));
	return affected;
}

void YDrawBuffer::verticalScroll( int delta ) {
	dbg() << "verticalScroll "<<delta<<" ; current mTopBufferLine = " << mTopBufferLine << endl;
	mTopBufferLine += delta;
	if ( delta < 0 ) {
		delta = qMin(mScreenHeight, -delta);
		int i = 0;
		for ( ; i < delta; ++i ) {
			mContent.insert(0, YDrawSection() << YDrawLine());
		}
		// remove out of screen lines
		for ( ; i < mContent.size() && delta < mScreenHeight; ++i ) {
			delta += mContent[i].count();
		}
		if ( delta >= mScreenHeight ) {
			while ( i < mContent.size() ) {
				mContent.takeAt(i);
			}
		}
	} else {
		for ( int i = 0; i < delta && mContent.size() > 0; ++i ) {
			mContent.takeAt(0);
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

void YDrawBuffer::setEOLCell( const YDrawCell& cell )
{
	mEOLCell = cell;
}

YDrawBufferIterator YDrawBuffer::iterator( const YInterval& i ) const
{
	return YDrawBufferIterator(this, i);
}

const YDrawSection YDrawBuffer::bufferDrawSection( int bl ) const
{
	int lid = bl - mTopBufferLine;
	YASSERT(0 <= lid)
	YASSERT(lid < mContent.count());
	return mContent[lid];
}
int YDrawBuffer::bufferDrawSectionScreenLine( int bl ) const
{
	int lid = bl - mTopBufferLine;
	YASSERT(0 <= lid)
	YASSERT(lid < mContent.count());
	int sl = 0;
	for ( int i = 0; i < lid; ++i ) {
		sl += mContent[i].count();
	}
	return sl;
}


/*************************
 * YDrawBufferIterator
 *************************/

YDrawBufferIterator::YDrawBufferIterator( const YDrawBuffer* db, const YInterval& i )
{
	mDrawBuffer = db;
	setup(i);
}
YDrawBufferIterator::~YDrawBufferIterator()
{
}

void YDrawBufferIterator::setup( const YInterval& i )
{
	mStopped = false;
	mI = i;
	int fLine = mI.fromPos().line();
	int fCol = mI.fromPos().column();
	if ( mI.from().opened() ) ++fCol;

	int dy = 0;
	mCurBLine = 0;
	int h = mDrawBuffer->mContent[mCurBLine].count();
	while ( mCurBLine < mDrawBuffer->mContent.count() && (dy + h) <= fLine ) {
		dy += h;
		++mCurBLine;
		h = mDrawBuffer->mContent[mCurBLine].count();
	}
	if ( mCurBLine >= mDrawBuffer->mContent.count() ) {
		mStopped = true;
	} else {
		mCurLine = fLine - dy;
		mCurCell = 0;
		int w = 0;
		bool found = false;
		mPos = YCursor(fCol, fLine);
		mNext.pos = mPos;
		foreach( YDrawCell cell, mDrawBuffer->mContent[mCurBLine][mCurLine].cells() ) {
			int cw = cell.c.length();
			if ( w + cw > fCol ) {
				cell.c = cell.c.mid(fCol - w);
				if ( fLine == mI.toPos().line() ) {
					/* take care of to() */
					int tCol = mI.toPos().column();
					if ( mI.to().opened() ) {
						--tCol;
						YASSERT(tCol >= 0);
					}
					cell.c = cell.c.left(tCol - fCol + 1);
				}
				mNext.type = YDrawCellInfo::Data;
				mNext.cell = cell;
				found = true;
				break;
			}
			w += cw;
			mCurCell += 1;
		}
		if ( !found ) {
			// EOL
			mNext.type = YDrawCellInfo::EOL;
			mNext.cell = mDrawBuffer->mEOLCell;
		}
	}
}

void YDrawBufferIterator::step()
{
	if ( mCurCell >= mDrawBuffer->mContent[mCurBLine][mCurLine].cells().count() ) {
		/* go to next line */
		++mCurLine;
		if ( mCurLine >= mDrawBuffer->mContent[mCurBLine].count() ) {
			/* go to next buffer line */
			++mCurBLine;
			mCurLine = 0;
			if ( mCurBLine >= mDrawBuffer->mContent.count() ) {
				/* getting out of screen */
				mStopped = true;
				return;
			}
		}
		mPos.setColumn(0);
		mPos.setLine(mPos.line() + 1);
		mCurCell = -1;
		mNext.cell = YDrawCell();
		step();
	} else {
		/* go to next cell */
		mPos.setColumn(mPos.column() + mNext.cell.c.length());
		if ( mPos > mI.toPos() || (mI.to().opened() && mPos >= mI.toPos()) ) {
			mStopped = true;
			return;
		}
		++mCurCell;
		mNext.pos = mPos;
		if ( mCurCell >= mDrawBuffer->mContent[mCurBLine][mCurLine].cells().count() ) {
			/* going out of line */
			if ( mPos.column() < mDrawBuffer->screenWidth() ) {
				/* EOL */
				mNext.type = YDrawCellInfo::EOL;
				mNext.cell = mDrawBuffer->mEOLCell;
			} else {
				step();
			}
		} else {
			YDrawCell cell = mDrawBuffer->mContent[mCurBLine][mCurLine].mCells[mCurCell];
			mNext.type = YDrawCellInfo::Data;
			if ( mPos.line() == mI.toPos().line() ) {
				/* take care of last line */
				int tCol = mI.toPos().column();
				if ( mI.to().opened() ) {
					--tCol;
					YASSERT(tCol >= 0);
				}
				cell.c = cell.c.left(tCol - mPos.column() + 1);
			}
			mNext.cell = cell;
		}
	}
}

bool YDrawBufferIterator::isValid() const
{
	return !mStopped;
}
void YDrawBufferIterator::next()
{
	YASSERT(isValid());
	step();
}

const YDrawCellInfo YDrawBufferIterator::drawCellInfo() const
{
	YASSERT(isValid());
	return mNext;
}
int YDrawBufferIterator::bufferLine() const
{
	YASSERT(isValid());
	return mDrawBuffer->topBufferLine() + mCurBLine;
}
int YDrawBufferIterator::screenLine() const
{
	YASSERT(isValid());
	return mPos.line();
}
int YDrawBufferIterator::lineHeight() const
{
	YASSERT(isValid());
	return mCurLine;
}

