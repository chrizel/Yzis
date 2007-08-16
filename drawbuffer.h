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
#include <QVector>

/* Yzis */
#include "color.h"
#include "font.h"
#include "selection.h"

class YView;

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

typedef QVector<YDrawCell> YDrawSection;
typedef QVector<YDrawSection> YDrawLine;

class YZIS_EXPORT YDrawBuffer
{

public:

    enum SetFromInfo {
        SetFromSeek,
    };

    YDrawBuffer();
    ~YDrawBuffer();

    void setCallback( YView* v );
    void setCallbackArgument( void* callback_arg );

    /* clear the buffer */
    void reset();

    void push( const QString& c );
    void newline( int y = -1 );
    void flush();

    void setFont( const YFont& f );
    void setColor( const YColor& c );
    void setBackgroundColor( const YColor& c );
    void setSelection( int sel );

    bool seek( const YCursor pos, YDrawBuffer::SetFromInfo sfi );

    YDrawCell at( const YCursor pos ) const;

    void replace( const YInterval& interval );

    /* Scroll dx to the right and dy downward */
    void Scroll( int dx, int dy );

    void setSelectionLayout( YSelectionPool::SelectionLayout layout, const YSelection& selection );

private :
    void insert_section( int pos = -1 );
    void insert_line( int pos = -1 );

    void push( const QChar& c );

    void callback( QPoint pos, const YDrawCell& cell );

    bool find( const YCursor pos, int* x, int* y, int* vx ) const;

    void applyPosition();

    /*
     * copy YColor @param c into YColor* @param dest.
     * Returns true if *dest has changed, false else
     */
    static bool updateColor( YColor* dest, const YColor& c );

    /* buffer content */
    YDrawLine m_content;

    /* current line */
    YDrawSection* m_line;
    /* current cell */
    YDrawCell* m_cell;

    /* current selection layouts */
    YSelectionLayout m_sel;

    int v_xi; /* column of the current section */
    int v_x; /* current draw column */

    int m_x; /* current section index */
    int m_y; /* current line index == current draw line */

    bool changed;
    YDrawCell m_cur;

    YView* m_view;
    void* m_callback_arg;

    friend YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );

};

YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );


#endif
