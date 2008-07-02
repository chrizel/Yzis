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

class YViewCursor;

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
}
;

class YDrawLine {
public :
	YDrawLine();
	~YDrawLine();

    void setFont( const YFont& f );
    void setColor( const YColor& c );
    void setBackgroundColor( const YColor& c );
	// TODO: setOutline
    void setSelection( int sel );

    int push( const QString& c );
	void flush();

	YViewCursor beginViewCursor() const;
	YViewCursor endViewCursor() const;

	YDrawSection arrange( int columns ) const;

	inline const QList<int> steps() const { return mSteps; }

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

	friend YDrawBuffer;
    friend YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl );
};
YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl );


typedef QList<YDrawLine> YDrawSection;

class YZIS_EXPORT YDrawBuffer
{

public:

    YDrawBuffer();
    ~YDrawBuffer();

	void setBufferDrawSection( int lid, YDrawSection ds );

private :
	QList<YDrawSection> mContent;

    friend YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );

};

YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );


#endif
