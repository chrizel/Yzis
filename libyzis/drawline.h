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

#ifndef DRAWLINE_H
#define DRAWLINE_H

#include <QList>

#include "yzis.h"
#include "color.h"
#include "font.h"
#include "drawbuffer.h"

class YDrawCell;


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
