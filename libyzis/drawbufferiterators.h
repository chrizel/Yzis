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

#ifndef DRAWBUFFERITERATORS_H
#define DRAWBUFFERITERATORS_H

#include "yzis.h"
#include "selection.h"
#include "drawcell.h"

class YDrawBuffer;
class YCursor;

struct YZIS_EXPORT YDrawCellInfo
{
	enum YDrawCellType {
		Data,
		EOL
	};

	YDrawCellType type;
	YCursor pos;
	YDrawCell cell;
};

class YZIS_EXPORT YDrawBufferAbstractIterator
{
public:
	virtual ~YDrawBufferAbstractIterator() {};

	/* TODO: docstring */
	bool isValid() const;
	/* TODO: docstring */
	void next();

	/* TODO: docstring */
	int bufferLine() const;
	/* TODO: docstring */
	int screenLine() const;
	/* TODO: docstring */
	int lineHeight() const;
	/* TODO: screenColumn */

protected:
	YDrawBufferAbstractIterator( YDrawBuffer* db );
	void setup( const YInterval& i, yzis::IntervalType itype );
	void step();

	virtual void setupCell( int cut ) = 0;
	virtual void setupEOLCell() = 0;

	int getCut();

	YDrawBuffer* mDrawBuffer;
	YInterval mI;
	yzis::IntervalType mIntervalType;
	bool mStopped;
	int mCurBLine;
	int mCurLine;
	int mCurCell;
	YCursor mPos;
	int mPosShift;
};

class YZIS_EXPORT YDrawBufferConstIterator : public YDrawBufferAbstractIterator
{
public:
	virtual ~YDrawBufferConstIterator() {}
	const YDrawCellInfo drawCellInfo() const;

protected :
	virtual void setupCell( int cut );
	virtual void setupEOLCell();
	YDrawBufferConstIterator( YDrawBuffer* db ) : YDrawBufferAbstractIterator(db) {}
	YDrawCellInfo mNext;

	friend class YDrawBuffer;
};

class YZIS_EXPORT YDrawBufferIterator : public YDrawBufferAbstractIterator
{
public:
	virtual ~YDrawBufferIterator() {}
	inline YDrawCell* cell() const { return mNext; }
	/* TODO: flush -> try to join splitted cells */

protected:
	virtual void setupCell( int cut );
	virtual void setupEOLCell();
	YDrawBufferIterator( YDrawBuffer* db ) : YDrawBufferAbstractIterator(db) {}
	YDrawCell* mNext;

	friend class YDrawBuffer;
};


#endif
