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
    bool valid;
    int flag;
    YFont font;
    QString c;
    YColor bg;
    YColor fg;
    int sel;
    YDrawCell():
            flag( 0 ),
            font(), c(), bg(), fg()
    {}
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

	bool hasNext();
	const YDrawCellInfo next();

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
	YCursor bufferBegin() const;

	/* TODO: docstring */
	YCursor bufferEnd() const;

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
	YInterval setBufferDrawSection( int lid, YDrawSection ds );

	/* TODO: docstring */
	void setEOLCell( const YDrawCell& cell );

private :
	QList<YDrawSection> mContent;

	YDrawCell mEOLCell;
	int mScreenWidth;
	int mScreenHeight;

    friend YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );

};

YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );

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

	YViewCursor beginViewCursor() const;
	YViewCursor endViewCursor() const;

	YDrawSection arrange( int columns ) const;

	inline const QList<int> steps() const { return mSteps; }
	inline const QList<YDrawCell> cells() const { return mCells; }

private:

    void insertCell( int pos = -1 );

    /*
     * copy YColor @param c into YColor* @param dest.
     * Returns true if *dest has changed, false else
     */
    static bool updateColor( YColor* dest, const YColor& c );

	void setLineCursor( int bufferY, int screenY );

	QList<YDrawCell> mCells;
	QList<int> mSteps;

	/* current cell */
    YDrawCell mCur;
    /* working cell */
    YDrawCell* mCell;

    bool changed;

	YViewCursor mBeginViewCursor;
	YViewCursor mEndViewCursor;

	friend class YDrawBuffer;
	friend class YDrawBufferIterator;
    friend YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl );
};
YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl );

#endif
