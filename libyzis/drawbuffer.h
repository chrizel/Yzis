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

#ifndef DRAWBUFFER_H
#define DRAWBUFFER_H

/* Qt */
#include <QList>

/* Yzis */
#include "color.h"
#include "font.h"
#include "selection.h"
#include "viewcursor.h"

class YDrawLine;
class YCursor;
typedef QList<YDrawLine> YDrawSection;

typedef QMap<YSelectionPool::SelectionLayout, YSelection> YSelectionLayout;

struct YDrawCell
{
    int flag;
    YFont font;
    QString c;
    YColor bg;
    YColor fg;
    int sel;
    YDrawCell() : flag(0), font(), c(), bg(), fg(), sel(0) {}
};

struct YDrawCellInfo
{
	enum YDrawCellType {
		Data,
		EOL
	};

	YDrawCellType type;
	YCursor pos;
	YDrawCell cell;
};

class YDrawBuffer;
class YZIS_EXPORT YDrawBufferIterator
{
public:
	YDrawBufferIterator( const YDrawBuffer* db, const YInterval& i );
	virtual ~YDrawBufferIterator();

	/* TODO: docstring */
	bool isValid() const;
	/* TODO: docstring */
	void next();

	/* TODO: docstring */
	const YDrawCellInfo drawCellInfo() const;
	/* TODO: docstring */
	int bufferLine() const;
	/* TODO: docstring */
	int screenLine() const;
	/* TODO: docstring */
	int lineHeight() const;

private:
	void setup( const YInterval& i );
	void step();

	const YDrawBuffer* mDrawBuffer;
	YInterval mI;
	YDrawCellInfo mNext;
	bool mStopped;
	int mCurBLine;
	int mCurLine;
	int mCurCell;
	YCursor mPos;
};

class YZIS_EXPORT YDrawBuffer
{
	friend class YDrawBufferIterator;

public:

    YDrawBuffer( int columns, int height );
    virtual ~YDrawBuffer();

	/* TODO: docstring */
	inline int topBufferLine() const {
		return mTopBufferLine;
	}
	/* TODO: docstring */
	inline int bottomBufferLine() const {
		return mTopBufferLine + mContent.size() - 1;
	}

	/* TODO: docstring */
	void verticalScroll( int delta );

	/* TODO: docstring */
	void setScreenSize( int columns, int lines );

	/* TODO: docstring */
	inline int screenHeight() const { return mScreenHeight; }

	/* TODO: docstring */
	inline int screenWidth() const { return mScreenWidth; }

	/* TODO: docstring */
	YDrawBufferIterator iterator( const YInterval& i ) const;

	/* TODO: docstring */
	inline const QList<YDrawSection> sections() { return mContent; }

	/* TODO: docstring */
	int setBufferDrawSection( int bl, YDrawSection ds, int* shift = NULL );

	/* TODO: docstring */
	YInterval deleteFromBufferDrawSection( int bl );

	/* TODO: docstring */
	void setEOLCell( const YDrawCell& cell );

	inline const YDrawCell EOLCell() const { return mEOLCell; };

	/* TODO: docstring */
	int currentHeight() const;

	inline bool full() const { return currentHeight() >= mScreenHeight; }

	/* TODO: docstring */
	const YDrawSection bufferDrawSection( int bl ) const;
	
	/* TODO: docstring */
	int bufferDrawSectionScreenLine( int bl ) const;

private :
	QList<YDrawSection> mContent;

	YCursor mScreenOffset;
	YDrawCell mEOLCell;
	int mScreenWidth;
	int mScreenHeight;
	int mTopBufferLine;

    friend YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );

};

extern YZIS_EXPORT YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );

class YZIS_EXPORT YDrawLine {
public :
	YDrawLine();
	virtual ~YDrawLine();

    void setFont( const YFont& f );
    void setColor( const YColor& c );
    void setBackgroundColor( const YColor& c );
	// TODO: setOutline
    void setSelection( int sel );

	void clear();

    int push( const QString& c );
	void flush();

	YDrawSection arrange( int columns ) const;

	inline const QList<int> steps() const { return mSteps; }
	inline const QList<YDrawCell> cells() const { return mCells; }

	inline int bufferLength() const { return mSteps.count(); }

private:

    void insertCell( int pos = -1 );

    /*
     * copy YColor @param c into YColor* @param dest.
     * Returns true if *dest has changed, false else
     */
    static bool updateColor( YColor* dest, const YColor& c );

	QList<YDrawCell> mCells;
	QList<int> mSteps;

	/* current cell */
    YDrawCell mCur;
    /* working cell */
    YDrawCell* mCell;

    bool changed;

	friend class YDrawBuffer;
	friend class YDrawBufferIterator;
    friend YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl );
};
extern YZIS_EXPORT YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl );

#endif
