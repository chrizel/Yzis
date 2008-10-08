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

#include <QList>

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


class YZIS_EXPORT YDrawBuffer
{
	friend class YDrawBufferAbstractIterator;
	friend class YDrawBufferConstIterator;
	friend class YDrawBufferIterator;

public:

    YDrawBuffer( const YView* view, int columns, int height );
    virtual ~YDrawBuffer();

	/* TODO: docstring */
	int screenTopBufferLine() const;

	/* TODO: docstring */
	int screenBottomBufferLine() const;

	/* TODO: docstring */
	void setScreenSize( int columns, int lines );

	/* TODO: docstring */
	inline int screenHeight() const { return mScreenHeight; }

	/* TODO: docstring */
	inline int screenWidth() const { return mScreenWidth; }

	inline int firstBufferLine() const { return mFirstBufferLine; }
	inline int lastBufferLine() const { return mFirstBufferLine+mContent.count()-1; }

	/* TODO: docstring */
	YDrawBufferConstIterator const_iterator( const YInterval& i, yzis::IntervalType itype );
	/* TODO: docstring */
	YDrawBufferIterator iterator( const YInterval& i, yzis::IntervalType itype );

	/* TODO: docstring */
	inline const QList<YDrawSection> sections() { return mContent; }

	/* TODO: docstring */
	YInterval setBufferDrawSection( int bl, YDrawSection ds );

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
	bool targetBufferLine( int bline, int* sid );
	/* TODO: docstring */
	int targetBufferColumn( int bcol, int sid, int* lid, int* cid, int* bshift, int* column = NULL ) const;
	/* TODO: docstring */
	bool targetScreenLine( int sline, int* sid, int* lid, int* bline = NULL ) const;
	/* TODO: docstring */
	int targetScreenColumn( int scol, int sid, int lid, int* cid, int* sshift, int* position = NULL ) const;

	/* TODO: docstring */
	bool scrollForViewCursor( const YViewCursor& vc, int* scrolling_horizontal, int* scroll_vertical );


private :
	QList<YDrawSection> mContent;

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
