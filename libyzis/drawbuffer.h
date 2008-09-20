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
#include "yzis.h"
#include "color.h"
#include "font.h"
#include "selection.h"
#include "viewcursor.h"

class YDrawLine;
class YCursor;
typedef QList<YDrawLine> YDrawSection;

class YDrawCell
{
public:
	YDrawCell();
	YDrawCell( const YDrawCell& cell );
	~YDrawCell();

	/* change properties for the whole cell */
	void addSelection( yzis::SelectionType selType );
	void delSelection( yzis::SelectionType selType );
	void setForegroundColor( const YColor& color );
	void setBackgroundColor( const YColor& color );
	void setFont( const YFont& font );
	void clear();
	
	int step( const QString& data );

	/* properties accessors */
	inline bool hasSelection( yzis::SelectionType selType ) const { return mSelections & selType; }
	inline YColor backgroundColor() const { return mColorBackground; }
	inline YColor foregroundColor() const { return mColorForeground; }
	inline YFont font() const { return mFont; }
	inline QString content() const { return mContent; };
	inline int width() const { return mContent.length(); };
	int widthForLength( int length ) const;
	int lengthForWidth( int width ) const;
	inline int length() const { return mSteps.count(); }

	/* steps (buffer <-> draw) */
	inline const QList<int> steps() const { return mSteps; }

	/* splitters */
	YDrawCell left( int column ) const;
	YDrawCell right( int column ) const;
	YDrawCell left_steps( int steps ) const;
	YDrawCell right_steps( int steps ) const;

private:
	int mSelections;
	YColor mColorForeground;
	YColor mColorBackground;
	YFont mFont;
	QString mContent;
	QList<int> mSteps;
	int mStepsShift;
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

class YDrawBufferAbstractIterator
{
public:
	virtual ~YDrawBufferAbstractIterator() {};

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
	/* TODO: screenColumn */

protected:
	YDrawBufferAbstractIterator( const YDrawBuffer* db );
	void setup( const YInterval& i, yzis::IntervalType itype );
	void step();

	virtual void setupCell( int shift, int cut ) = 0;
	virtual void setupEOLCell() = 0;

	int getCut();

	const YDrawBuffer* mDrawBuffer;
	YInterval mI;
	yzis::IntervalType mIntervalType;
	bool mStopped;
	int mCurBLine;
	int mCurLine;
	int mCurCell;
	YCursor mPos;
};

class YZIS_EXPORT YDrawBufferConstIterator : public YDrawBufferAbstractIterator
{
public:
	virtual ~YDrawBufferConstIterator() {}
	const YDrawCellInfo drawCellInfo() const;

protected :
	virtual void setupCell( int shift, int cut );
	virtual void setupEOLCell();
	YDrawBufferConstIterator( const YDrawBuffer* db ) : YDrawBufferAbstractIterator(db) {}
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
	YDrawBuffer* mDrawBuffer;
	virtual void setupCell( int shift, int cut );
	virtual void setupEOLCell();
	YDrawBufferIterator( const YDrawBuffer* db ) : YDrawBufferAbstractIterator(db) {}
	YDrawCell* mNext;

	friend class YDrawBuffer;
};


class YZIS_EXPORT YDrawBuffer
{
	friend class YDrawBufferAbstractIterator;
	friend class YDrawBufferConstIterator;
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
	YDrawBufferConstIterator const_iterator( const YInterval& i, yzis::IntervalType itype ) const;
	/* TODO: docstring */
	YDrawBufferIterator iterator( const YInterval& i, yzis::IntervalType itype );

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

	/* TODO: docstring */
	YInterval addSelection( yzis::SelectionType sel, const YInterval& i, yzis::IntervalType itype );
	/* TODO: docstring */
	YInterval delSelection( yzis::SelectionType sel, const YInterval& i, yzis::IntervalType itype );

	/* TODO: docstring */
	bool targetBufferLine( int bline, int* sid ) const;
	/* TODO: docstring */
	bool targetBufferColumn( int bcol, int sid, int* lid, int* cid, int* bshift, int* column = NULL ) const;
	/* TODO: docstring */
	bool targetScreenLine( int sline, int* sid, int* lid ) const;
	/* TODO: docstring */
	bool targetScreenColumn( int scol, int sid, int lid, int* cid, int* sshift, int* position = NULL ) const;


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

class YZIS_EXPORT YDrawLine : public QList<YDrawCell> {
public :
	YDrawLine();
	virtual ~YDrawLine();

    void setFont( const YFont& f );
    void setColor( const YColor& c );
    void setBackgroundColor( const YColor& c );
	// TODO: setOutline

	void clear();

    int step( const QString& c );
	void flush();

	YDrawSection arrange( int columns ) const;

	inline const int width() const { return mWidth; }
	inline int length() const { return mLength; }

private:

	/* current cell */
    YDrawCell mCur;
    /* working cell */
    YDrawCell* mCell;

	int mWidth;
	int mLength;

    bool changed;

	friend class YDrawBuffer;
	friend class YDrawBufferIterator;
    friend YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl );
};
extern YZIS_EXPORT YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl );

#endif
