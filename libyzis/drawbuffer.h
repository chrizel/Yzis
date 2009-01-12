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

/*********
 *
 * YDrawBuffer is the data structure used to abstract the screen rendering.
 *
 * DrawBuffer is divided into sections. To each section corresponds one and only one buffer line.
 * A section is divided into lines, themselves divided into cells. A cell
 * displays several characters sharing the same properties (color, fonts, etc.)
 *
 * ##################################################  section 0.
 * |            |   |      |        |            |  |  line 0.
 * --------------------------------------------------
 * |   |         |         |                           line 1.
 * ##################################################  section 1.
 * |     |          |              |     | | | |  | |  line 0.
 * --------------------------------------------------
 * | | | | | |    | | |      | | |   | |           ||  line 1.
 * --------------------------------------------------
 * |         |                     |   | | |           line 2.
 *  ...
 *
 *
 ***********/

/* Qt */
#include <QList>

/* Yzis */
#include "yzis.h"
#include "selection.h"
#include "drawbufferiterators.h"

class YDrawLine;
typedef QList<YDrawLine> YDrawSection;

#include "drawline.h"


class YDrawCell;
class YCursor;
class YView;
class YViewCursor;

struct YDrawLineLock {
	int line;
	int count;
	YDrawLineLock( int l, int c ) {
		line = l;
		count = c;
	}
};


class YZIS_EXPORT YDrawBuffer
{
	friend class YDrawBufferAbstractIterator;
	friend class YDrawBufferConstIterator;
	friend class YDrawBufferIterator;

public:

    YDrawBuffer( const YView* view, int columns, int height );
    virtual ~YDrawBuffer();

	/*
	 * Returns the buffer line number corresponding to
	 * the on screen top line
	 */
	int screenTopBufferLine() const;

	/**
	 * Returns the buffer line number corresponding to
	 * the on screen bottom line
	 */
	int screenBottomBufferLine() const;

	/**
	 * Sets the screen width and height fields.
	 */
	void setScreenSize( int columns, int lines );

	/**
	 * Returns the height of the screen 
	 */
	inline int screenHeight() const { return mScreenHeight; }

	/**
	 * Returns the width of the screen
	 */
	inline int screenWidth() const { return mScreenWidth; }

	/**
	 * Returns the first in-memory buffer line
	 */
	inline int firstBufferLine() const { return mFirstBufferLine; }

	/**
	 * Returns the last in-memory buffer line
	 */
	inline int lastBufferLine() const { return mFirstBufferLine+mContent.count()-1; }

	/**
	 * Returns an iterator on this drawbuffer for the given interval @arg i
	 * screen or buffer mode is given trought @arg itype
	 * This iterator is suitable for read-only operations on the drawbuffer
	 * see YDrawBufferConstIterator
	 */
	YDrawBufferConstIterator const_iterator( const YInterval& i, yzis::IntervalType itype );

	/**
	 * Returns an iterator on this drawbuffer for the given interval @arg i
	 * screen or buffer mode is given trought @arg itype
	 * This iterator is suitable for read and write operations on the drawbuffer
	 * see YDrawBufferIterator
	 */
	YDrawBufferIterator iterator( const YInterval& i, yzis::IntervalType itype );

	/**
	 * Accessor to the in-memory lines
	 */
	inline const QList<YDrawSection> sections() { return mContent; }

	/**
	 * Sets the draw sections corresponding the buffer line @arg bl.
	 * Returns the screen part affected by the change.
	 */
	YInterval setBufferDrawSection( int bl, YDrawSection ds );

	/**
	 * Remove the buffer line @arg bl.
	 * Returns the screen part affected by the change.
	 */
	YInterval deleteFromBufferDrawSection( int bl );

	/**
	 * Sets the YDrawCell used to draw the end of lines.
	 */
	void setEOLCell( const YDrawCell& cell );

	/**
	 * Get the YDrawCell used to draw the end of lines.
	 */
	inline const YDrawCell EOLCell() const { return mEOLCell; };

	/**
	 * Compute the current height used by lines on screen.
	 */
	int currentHeight() const;

	/**
	 * Returns true if the screen is filled of lines. False if there is space.
	 */
	inline bool full() const { return currentHeight() >= mScreenHeight; }

	/**
	 * Access to the draw section of @arg bl buffer line.
	 */
	const YDrawSection bufferDrawSection( int bl ) const;
	
	/**
	 * Compute the screen line corresponding to the buffer line @arg bl.
	 */
	int bufferDrawSectionScreenLine( int bl ) const;

	/**
	 * Mark the given @arg i interval as selected by @arg sel.
	 * Returns the affected part of screen.
	 */
	YInterval addSelection( yzis::SelectionType sel, const YInterval& i, yzis::IntervalType itype );

	/**
	 * Remove @arg sel selection mark from the given @arg i interval.
	 * Returns the affected part of screen.
	 */
	YInterval delSelection( yzis::SelectionType sel, const YInterval& i, yzis::IntervalType itype );

	/**
	 * Low-level method.
	 * Computes the corresponding section index of @arg bline buffer line for the drawbuffer data structure (may ask the view for missing lines, if any). Stores the result in @arg sid.
	 * Returns true unless the bline is not accessible.
	 *
	 * Warning: do not use this method, unless you known what you are doing. Please use iterators instead.
	 */
	bool targetBufferLine( int bline, int* sid );

	/**
	 * Low-level method.
	 * Computes the corresponding indexes of @arg bcol buffer column for the drawbuffer data structure. It needs an already computed index @arg sid corresponding to the section index.
	 * Line index (relative to the section index @arg sid) is stored into @arg lid.
	 * Cell index inside the YDrawLine is stored into @arg cid.
	 * @arg bshift corresponds to the shift to be done inside the cell.
	 * If @arg column is not null, the corresponding screen column will be stored into it.
	 *
	 * Returns the accessed buffer column.
	 *
	 * Warning: do not use this method, unless you known what you are doing. Please use iterators instead.
	 */
	int targetBufferColumn( int bcol, int sid, int* lid, int* cid, int* bshift, int* column = NULL ) const;

	/**
	 * Low-level method.
	 * Computes the corresponding section index of @arg sline screen line for the drawbuffer data structure.
	 * Section index is stored into @arg sid.
	 * Line index (for the section) is stored into @arg lid.
	 * If @arg bline is not null, the corresponding buffer line will be stored into it.
	 * Returns true unless @arg sline is not accessible.
	 *
	 * Warning: do not use this method, unless you known what you are doing. Please use iterators instead.
	 */
	bool targetScreenLine( int sline, int* sid, int* lid, int* bline = NULL ) const;

	/**
	 * Low-level method.
	 * Computes the corresponding indexes of @arg scol screen column for the drawbuffer data structure. It needs already computed section and line indexes.
	 * Cell index (for the line) is stored into @arg cid.
	 * @arg sshift will contain the shift to be done from the cell.
	 * If @arg position is not null, the corresponding buffer column will be stored into it.
	 * 
	 * Returns the accessed screen column.
	 *
	 * Warning: do not use this method, unless you known what you are doing. Please use iterators instead.
	 */
	int targetScreenColumn( int scol, int sid, int lid, int* cid, int* sshift, int* position = NULL ) const;


	/**
	 * Make the drawbuffer scroll to ensure the YViewCursor @arg vc is visible.
	 * The horizontal delta hd is stored into @arg scrolling_horizontal.
	 * The vertical delta vd is stored into @arg scrolling_vertical.
	 * Returns true unless no scrolling is needed.
	 *
	 * The row v has to be shifted to row v-vd.
	 * The column c has to be shifted to column c-hd.
	 */
	bool scrollForViewCursor( const YViewCursor& vc, int* scrolling_horizontal, int* scroll_vertical );

	/**
	 * Make the drawbuffer scroll to ensure the line @arg line is the on-screen top line.
	 * see scrollForViewCursor.
	 */
	bool scrollLineToTop( int line, int* scrolling_horizontal, int* scroll_vertical );

	/**
	 * Make the drawbuffer scroll to ensure the line @arg line is the on-screen center line.
	 * see scrollForViewCursor.
	 */
	bool scrollLineToCenter( int line, int* scrolling_horizontal, int* scroll_vertical );

	/**
	 * Make the drawbuffer scroll to ensure the line @arg line is the on-screen bottom line.
	 * see scrollForViewCursor.
	 */
	bool scrollLineToBottom( int line, int* scrolling_horizontal, int* scroll_vertical );


	/**
	 * Locks the buffer line @line
	 *  drawbuffer will keep into memory all lines from the locked lines
	 * 	to the displayed ones 
	 */
	void acquireLine( int line );

	/**
	 * Unlocks the buffer line @line
	 *  this line may be removed from memory
	 */
	void releaseLine( int line );

	/**
	 * Remove unneeded lines from memory.
	 */
	void squeeze();


private :
	QList<YDrawSection> mContent;

	QList<YDrawLineLock> mLocks;

	YCursor mScreenOffset;
	YDrawCell mEOLCell;
	int mScreenWidth;
	int mScreenHeight;
	int mFirstBufferLine;
	int mScreenTopBufferLine;
	
	const YView* mView;

    friend YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );

};

extern YZIS_EXPORT YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );


#endif
