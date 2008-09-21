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

#include "drawbufferiterators.h"
#include "drawbuffer.h"
#include "drawline.h"
#include "drawcell.h"
#include <QList>

#include "debug.h"
#define dbg()    yzDebug("YDrawBufferIterators")
#define err()    yzError("YDrawBufferIterators")


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


