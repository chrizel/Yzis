/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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

#ifndef YZIS_CURSOR
#define YZIS_CURSOR

#include <QPoint>
#include "yzismacros.h" // YZIS_EXPORT

class YDebugStream;

/**
  * @short Store a cursor position
  *
  * An YCursor is nothing more than a QPoint with some more methods needed
  * by Yzis.
  *
  * The order used for comparison is first on y and then on x.
  * With: x1 < x2, y1 < y2
  * we get: 
  * YCursor( x1, y1 ) < YCursor( x2, y1 )
  * YCursor( x2, y1 ) < YCursor( x1, y2 )
  *
  */
class YZIS_EXPORT YCursor : public QPoint
{

public :
    YCursor() : QPoint( -1, -1)
    { }
    YCursor( const QPoint &p ) : QPoint(p)
    { } // handles YCursor(const YCursor&) as well
    YCursor(const int x, const int y) : QPoint(x, y)
    { }

    void setXY( int x, int y )
    {
        setX(x); setY(y);
    }
    void setLineColumn( int line, int col )
    {
        setX(col); setY(line);
    }

    // aliases
    int column() const
    {
        return x();
    }
    int line() const
    {
        return y();
    }

    // TODO : we should add a QString cast to help when we want
    // to output with QString.arg().

    /*
     * operators
     */
    bool operator< ( const YCursor right ) const;
    bool operator<= ( const YCursor right ) const;
    bool operator> ( const YCursor right ) const;
    bool operator>= ( const YCursor right ) const;

};

extern YZIS_EXPORT YDebugStream &operator<< ( YDebugStream & out, const YCursor c );

#endif

