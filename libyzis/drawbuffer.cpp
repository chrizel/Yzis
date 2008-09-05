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
YDrawCell YDrawCell::right_steps( int steps ) const
{
	YDrawCell c(*this);
	if ( steps > 0 ) {
		c.mStepsShift = 0;
		int w = mStepsShift;
		for ( int i = 0; i < steps; i++ ) {
			w += mSteps[i];
		}
		c.mContent = mContent.right(w);
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

YInterval YDrawBuffer::addSelection( yzis::SelectionType sel, const YInterval& i, yzis::IntervalType itype )
{
	YDrawBufferIterator it = iterator(i, itype);
	if ( !it.isValid() ) {
		return YInterval();
	}
	YCursor begin(/*TODO*/0, it.screenLine());
	int lastScreenLine = 0;
	int lastScreenColumn = screenWidth(); /*TODO*/
	for ( ; it.isValid(); it.next() ) {
		it.cell()->addSelection(sel);
		lastScreenLine = it.screenLine();
		/*lastScreenColumn = it.screenColumn() + it.cell()->width();*/
	}
	return YInterval(begin, YCursor(lastScreenColumn, lastScreenColumn));
}

YInterval YDrawBuffer::delSelection( yzis::SelectionType sel, const YInterval& i, yzis::IntervalType itype )
{
	YDrawBufferIterator it = iterator(i, itype);
	if ( !it.isValid() ) {
		return YInterval();
	}
	YCursor begin(/*TODO*/0, it.screenLine());
	int lastScreenLine = 0;
	int lastScreenColumn = screenWidth(); /*TODO*/
	for ( ; it.isValid(); it.next() ) {
		it.cell()->delSelection(sel);
		lastScreenLine = it.screenLine();
		/*lastScreenColumn = it.screenColumn() + it.cell()->width();*/
	}
	return YInterval(begin, YCursor(lastScreenColumn, lastScreenColumn));
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

YDrawBufferConstIterator YDrawBuffer::const_iterator( const YInterval& i, yzis::IntervalType itype ) const
{
	YDrawBufferConstIterator it = YDrawBufferConstIterator(this);
	it.setup(i, itype);
	return it;
}
YDrawBufferIterator YDrawBuffer::iterator( const YInterval& i, yzis::IntervalType itype )
{
	YDrawBufferIterator it = YDrawBufferIterator(this);
	it.setup(i, itype);
	return it;
}

const YDrawSection YDrawBuffer::bufferDrawSection( int bl ) const
{
	int sid = bl - mTopBufferLine;
	YASSERT(0 <= sid)
	YASSERT(sid < mContent.count());
	return mContent[sid];
}
int YDrawBuffer::bufferDrawSectionScreenLine( int bl ) const
{
	int sid = bl - mTopBufferLine;
	YASSERT(0 <= sid)
	YASSERT(sid < mContent.count());
	int sl = 0;
	for ( int i = 0; i < sid; ++i ) {
		sl += mContent[i].count();
	}
	return sl;
}

bool YDrawBuffer::targetBufferLine( int bline, int* sid ) const
{
	YASSERT(mTopBufferLine <= bline);
	YASSERT(bline - mTopBufferLine < mContent.count());
	*sid = bline - mTopBufferLine;
	return true;
}
bool YDrawBuffer::targetBufferColumn( int bcol, int sid, int* lid, int* cid, int* bshift ) const
{
	YASSERT(0 <= bcol);
	bool lid_found = false;
	int w = 0;
	int my_lid = 0;
	for( ; !lid_found && my_lid < mContent[sid].count(); ++my_lid ) {
		int lw = mContent[sid][my_lid].length();
		if ( w + lw > bcol ) {
			lid_found = true;
			break;
		} else {
			w += lw;
		}
	}
	bool found = false;
	if ( lid_found ) {
		int my_cid = 0;
		for( ; !found && my_cid < mContent[sid][my_lid].count(); ++my_cid ) {
			int cw = mContent[sid][my_lid][my_cid].length();
			if ( w + cw > bcol ) {
				*bshift = bcol - w;
				found = true;
			} else {
				w += cw;
			}
		}
		*lid = my_lid;
		*cid = my_cid;
	}
	return found;
}

bool YDrawBuffer::targetScreenLine( int sline, int* sid, int* lid ) const
{
	YASSERT(0 <= sline);
	YASSERT(sline < screenHeight());
	bool found = false;
	int my_sid = 0;
	int my_lid = 0;
	int h = 0;
	for ( ; !found && my_sid < mContent.count(); ++my_sid ) {
		int sh = mContent[my_sid].count();
		if ( h + sh > sline ) {
			my_lid = sline - h;
			found = true;
		} else {
			h += sh;
		}
	}
	YASSERT(0 <= my_lid);
	YASSERT(my_lid < mContent[my_sid].count());
	*sid = my_sid;
	*lid = my_lid;
	return found;
}
bool YDrawBuffer::targetScreenColumn( int scol, int sid, int lid, int* cid, int* sshift ) const
{
	YASSERT(0 <= scol);
	YASSERT(scol < screenWidth());
	bool found = false;
	int my_cid = 0;
	int w = 0;
	for( ; !found && my_cid < mContent[sid][lid].count(); ++my_cid ) {
		int cw = mContent[sid][lid][my_cid].width();
		if ( w + cw > scol ) {
			*sshift = scol - w;
			found = true;
		} else {
			w += cw;
		}
	}
	*cid = my_cid;
	return found;
}


/*************************
 * YDrawBufferIterator
 *************************/

YDrawBufferAbstractIterator::YDrawBufferAbstractIterator( const YDrawBuffer* db )
{
	mDrawBuffer = db;
}
int YDrawBufferAbstractIterator::bufferLine() const
{
	YASSERT(isValid());
	return mDrawBuffer->topBufferLine() + mCurBLine;
}
int YDrawBufferAbstractIterator::screenLine() const
{
	YASSERT(isValid());
	return mPos.line();
}
int YDrawBufferAbstractIterator::lineHeight() const
{
	YASSERT(isValid());
	return mCurLine;
}
bool YDrawBufferAbstractIterator::isValid() const
{
	return !mStopped;
}
void YDrawBufferAbstractIterator::next()
{
	YASSERT(isValid());
	step();
}

void YDrawBufferAbstractIterator::setup( const YInterval& i, yzis::IntervalType itype )
{
	mI = i;
	mIntervalType = itype;
	mStopped = false;

	YCursor start = i.closedStartCursor();
	bool found = false;
	switch ( mIntervalType ) {
		case yzis::ScreenInterval :
			found = mDrawBuffer->targetScreenLine(start.line(), &mCurBLine, &mCurLine);
			break;
		case yzis::BufferInterval :
			found = mDrawBuffer->targetBufferLine(start.line(), &mCurBLine);
			break;
	}
	if ( !found ) {
		mStopped = true;
	} else {
		int shift = 0;
		mPos = start;
		switch ( mIntervalType ) {
			case yzis::ScreenInterval :
				found = mDrawBuffer->targetScreenColumn(start.column(), mCurBLine, mCurLine, &mCurCell, &shift);
				break;
			case yzis::BufferInterval :
				found = mDrawBuffer->targetBufferColumn(start.column(), mCurBLine, &mCurLine, &mCurCell, &shift);
				break;
		}
		if ( found ) {
			setupCell(shift, getCut());
		} else {
			setupEOLCell();
		}
	}
}

int YDrawBufferAbstractIterator::getCut() {
	int cut = 0;
	if ( mPos.line() == mI.toPos().line() ) {
		YCursor end = mI.openedEndCursor();
		YDrawCell cell = mDrawBuffer->mContent[mCurBLine][mCurLine][mCurCell];
		int w = 0;
		switch ( mIntervalType ) {
			case yzis::ScreenInterval :
				w = cell.width();
				break;
			case yzis::BufferInterval :
				w = cell.length();
				break;
		}
		if ( mPos.column() + w >= end.column() ) {
			cut = end.column() - mPos.column();
		}
	}
	return cut;
}

void YDrawBufferAbstractIterator::step()
{
	if ( mCurCell >= mDrawBuffer->mContent[mCurBLine][mCurLine].count() ) {
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
			if ( mIntervalType == yzis::BufferInterval ) {
				mPos.setColumn(0);
				mPos.setLine(mPos.line() + 1);
			}
		}
		if ( mIntervalType == yzis::ScreenInterval ) {
			mPos.setColumn(0);
			mPos.setLine(mPos.line() + 1);
		}
		mCurCell = -1;
		step();
	} else {
		/* go to next cell */
		if ( mCurCell >= 0 ) {
			switch( mIntervalType ) {
				case yzis::ScreenInterval:
					mPos.setColumn(mPos.column() + mDrawBuffer->mContent[mCurBLine][mCurLine][mCurCell].width());
					break;
				case yzis::BufferInterval:
					mPos.setColumn(mPos.column() + mDrawBuffer->mContent[mCurBLine][mCurLine][mCurCell].length());
					break;
			}
		}
		if ( mPos > mI.toPos() || (mI.to().opened() && mPos >= mI.toPos()) ) {
			/* STOP */
			return;
		} else {
			++mCurCell;
			if ( mCurCell >= mDrawBuffer->mContent[mCurBLine][mCurLine].count() ) {
				/* going out of drawline */
				if ( mIntervalType == yzis::ScreenInterval && mPos.column() < mDrawBuffer->screenWidth() ) {
					setupEOLCell();
				} else {
					step();
				}
			} else {
				setupCell(0, getCut());
			}
		}
	}
}


/*
 * ConstIterator
 */
void YDrawBufferConstIterator::setupCell( int shift, int cut )
{
	mNext.pos = mPos;
	mNext.type = YDrawCellInfo::Data;
	YDrawCell cell = mDrawBuffer->mContent[mCurBLine][mCurLine][mCurCell];
	if ( shift ) {
		switch ( mIntervalType ) {
			case yzis::ScreenInterval :
				cell = cell.right(shift);
				break;
			case yzis::BufferInterval :
				cell = cell.right_steps(shift);
				break;
		}
	}
	if ( cut ) {
		switch ( mIntervalType ) {
			case yzis::ScreenInterval :
				cell = cell.left(cut);
				break;
			case yzis::BufferInterval :
				cell = cell.left_steps(cut);
				break;
		}
	}
	mNext.cell = cell;
}
void YDrawBufferConstIterator::setupEOLCell()
{
	mNext.pos = mPos;
	mNext.type = YDrawCellInfo::EOL;
	mNext.cell = mDrawBuffer->mEOLCell;
}
const YDrawCellInfo YDrawBufferConstIterator::drawCellInfo() const
{
	YASSERT(isValid());
	return mNext;
}

/*
 * Iterator
 */

void YDrawBufferIterator::setupCell( int shift, int cut )
{
	if ( shift || cut ) {
		YDrawCell cell = mDrawBuffer->mContent[mCurBLine][mCurLine][mCurCell];
		YDrawCell previous;
		if ( shift ) {
			switch ( mIntervalType ) {
				case yzis::ScreenInterval :
					previous = cell.left(shift);
					cell = cell.right(shift);
					break;
				case yzis::BufferInterval :
					previous = cell.left_steps(shift);
					cell = cell.right_steps(shift);
					break;
			}
			mDrawBuffer->mContent[mCurBLine][mCurLine][mCurCell] = previous;
			++mCurCell;
			mDrawBuffer->mContent[mCurBLine][mCurLine].insert(mCurCell, cell);
		}
		if ( cut ) {
			YDrawCell next;
			switch ( mIntervalType ) {
				case yzis::ScreenInterval :
					next = cell.right(cut);
					cell = cell.left(cut);
					break;
				case yzis::BufferInterval :
					next = cell.right_steps(cut);
					cell = cell.left_steps(cut);
					break;
			}
			mDrawBuffer->mContent[mCurBLine][mCurLine][mCurCell] = cell;
			mDrawBuffer->mContent[mCurBLine][mCurLine].insert(mCurCell+1, next);
		}
	}
	mNext = &mDrawBuffer->mContent[mCurBLine][mCurLine][mCurCell];
}
void YDrawBufferIterator::setupEOLCell()
{
	/* ignore them */
	step();
}

