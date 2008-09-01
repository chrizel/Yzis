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

YDrawCell YDrawCell::left( int column ) const
{
	YDrawCell l(*this);
	if ( column < width() ) {
		l.mContent = mContent.left(column);
		l.mSteps.clear();
		int w = 0;
		int r = column;
		foreach( int s, mSteps ) {
			if ( r == 0 ) break;
			if ( s > r ) {
				l.mSteps << r;
				r = 0;
			} else {
				l.mSteps << s;
				w += s;
				r -= s;
			}
		}
	}
	return l;
}

YDrawCell YDrawCell::right( int column ) const
{
	YDrawCell c(*this);
	if ( column > 0 ) {
		c.mContent = mContent.right(column);
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



/************************
 * YDrawLine
 ************************/

YDrawLine::YDrawLine() :
	mCells()
{
	clear();
}
YDrawLine::~YDrawLine()
{
}

void YDrawLine::clear() {
	mCur.clear();
	mCell = NULL;
	mCells.clear();
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
		mCells << YDrawCell(mCur);
		mCell =& mCells[mCells.size()-1];
		changed = false;
	}
	return mCell->step(c);
}
void YDrawLine::flush()
{
	mWidth = 0;
	/* TODO */
}

YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl )
{
	for ( int i = 0; i < dl.mCells.size(); ++i ) {
		out << "'" << dl.mCells[i].content() << "' ";
	}
    return out;
}

YDrawSection YDrawLine::arrange( int columns ) const
{
	if ( mCells.count() == 0 ) {
		return YDrawSection() << YDrawLine();
	}

	YDrawSection ds;
	QList<YDrawCell> line;
	int line_width = 0;

	foreach( YDrawCell c, mCells ) {
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
			dl.mCells = line;
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
		dl.mCells = line;
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
	mEOLCell.step(" ");
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

YInterval YDrawBuffer::addBufferSelection( yzis::SelectionType type, const YInterval& bufferInterval )
{
	int lastLine = bufferInterval.toPos().line();
	if ( bufferInterval.to().opened() && bufferInterval.toPos().column() == 0 ) {
		--lastLine;
	}
	int firstLine = qMax(bufferInterval.fromPos().line(), mDrawBuffer.topBufferLine());
	if ( firstLine > lastLine ) return; // out of screen selection

	firstLine = qMax(firstLine - mTopBufferLine, 0);
	lastLine = qMin(mContent.size()-1, lastLine-mTopBufferLine);

	if ( firstLine > lastLine ) { 
		/* out of screen */
		return YInterval();
	}

	int sl = 0;
	for ( int i = 0; i < firstLine; ++i ) {
		sl += mContent[i].count();
	}
	YCursor begin(0, sl);
	for ( int i = firstLine; i < lastLine; ++i ) {
		int j = 0;
		for ( ; j < mContent[i].count(); ++j ) {
			mContent[i][j].addSelection(type, x1, x2);
		}
		sl += j;
	}
	return YInterval(begin, YBound(YCursor(0, sl), true));
}

YInterval YDrawBuffer::removeBufferSelection( yzis::SelectionType type, const YInterval& bufferInterval )
{
	//first affected cell
	//last affected cell
	//- selection
	//try to join first and last cell to previous and following
	return YInterval();
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
			int cw = cell.width();
			if ( w + cw > fCol ) {
				cell = cell.right(fCol - w);
				if ( fLine == mI.toPos().line() ) {
					/* take care of to() */
					int tCol = mI.toPos().column();
					if ( mI.to().opened() ) {
						--tCol;
						YASSERT(tCol >= 0);
					}
					cell = cell.left(tCol - fCol + 1);
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
		mPos.setColumn(mPos.column() + mNext.cell.width());
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
				cell = cell.left(tCol - mPos.column() + 1);
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

