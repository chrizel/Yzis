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
#include <QString>
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
    YCursor() : QPoint(0, 0)
    { }
    YCursor( const QPoint &p ) : QPoint(p)
    { } // handles YCursor(const YCursor&) as well
    YCursor(const int x, const int y) : QPoint(x, y)
    { }

    void setXY( int x, int y )
    {
        setX(x); setY(y);
    }

	/* aliases : line is y, column is x */
	inline void setLine( int line ) { setY(line); }
	inline int line() const { return y(); }
	inline void setColumn( int column ) { setX(column); }
	inline int column() const { return x(); }
	inline void setLineColumn( int line, int column ) { setXY(column, line); }

    QString toString() const
    {
        return QString("(line,col)=(%1,%2)").arg(line()).arg(column());
    }

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

