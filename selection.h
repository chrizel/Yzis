/*  This file is part of the Yzis libraries
*  Copyright (C) 2004-2005 Loic Pauleve <panard@inzenet.org>
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

#ifndef YZ_SELECTION_H
#define YZ_SELECTION_H

#include <QMap>
#include <QString>
#include <QRect>

#include "cursor.h" 
/**
 * Boundary of an interval that either belongs to it or not
 *
 * Panard's explanation in an email to yzis-dev:
 * I introduced bounds to remove ambiguity with the use of intervals :
 * open and close correspond to (x,y)[ (open) or (x,y)] (close) then when defining
 * intervals, you can do   [(x1,y1), (x2,y2)[,  [(x1,y1), (x2,y2)], etc...  In
 * many methods (copy, paste, selections, rendering, etc...), the 'from' and 'to'
 * positions are sometimes included and sometimes excluded.  Using an YBound make
 * it explicit.
 
 * Another use is, by example, in visual mode : [(0,0), (0,1)[ will select line 0
 * entirely without have to calculate line length.
 */
class YZIS_EXPORT YBound
{
public:
    YBound( const YBound& bound ) : mPos( bound.pos() )
    {
        mOpen = bound.opened();
    }
    explicit YBound( const YCursor pos, bool open = false ) : mPos( pos )
    {
        mOpen = open;
    }
    YBound()
    {}

    void setPos( const YCursor pos )
    {
        mPos = pos;
    }
    void open();
    void close();
    const YCursor pos() const;
    bool opened() const;
    bool closed() const;

private:
    YCursor mPos;
    bool mOpen;
};

bool operator==( const YBound& left, const YBound& right );
bool operator>( const YBound& left, const YBound& right );
bool operator<( const YBound& left, const YBound& right );
bool operator>=( const YBound& left, const YBound& right );
bool operator<=( const YBound& left, const YBound& right );
bool operator>=( const YBound& left, const YCursor right );
bool operator<=( const YBound& left, const YCursor right );
bool operator>=( const YCursor left, const YBound& right );
bool operator<=( const YCursor left, const YBound& right );
const YBound operator-( const YBound& left, const YCursor right );

/**
 * An interval between two @ref YBound "YBounds"
 *
 * It can also be constructed from @ref YCursor "YCursors".
 */
class YZIS_EXPORT YInterval
{

    friend YDebugStream& operator<<( YDebugStream& out, const YInterval& i );

public:
    YInterval( const YBound& from, const YBound& to ) : mFrom( from ), mTo( to )
    {}
    YInterval( const YCursor from, const YCursor to ) : mFrom( from ), mTo( to )
    {}
    YInterval( const YBound& from, const YCursor to ) : mFrom( from ), mTo( to )
    {}
    YInterval( const YCursor from, const YBound& to ) : mFrom( from ), mTo( to )
    {}
    YInterval( const QRect& r ) : mFrom( YCursor(r.left(), r.top()) ), mTo( YCursor(r.right(), r.bottom()) )
    {}
    YInterval()
    {}

    void setFrom( const YBound& bound );
    void setTo( const YBound& bound );
    const YBound& from() const;
    const YBound& to() const;

    void setFromPos( const YCursor pos );
    void setToPos( const YCursor pos );
    const YCursor fromPos() const;
    const YCursor toPos() const;

    bool contains( const YCursor pos ) const;
    bool contains( const YInterval& pos ) const;

    QRect boundingRect() const;

private:
    YBound mFrom;
    YBound mTo;
};
YZIS_EXPORT const YInterval operator-( const YInterval& l, const YCursor r );

typedef QMap<unsigned int, YInterval> YSelectionMap;

/**
 * Holds a selection
 */
class YZIS_EXPORT YSelection
{

    friend YZIS_EXPORT YDebugStream& operator<<( YDebugStream& out, const YSelection& s );

public:
    YSelection();
    YSelection( const QString& name );
    YSelection( const YInterval& i );

    YSelectionMap map() const;
    void setMap( const YSelectionMap& m );

    void addMap( const YSelectionMap& m );
    void addInterval( const YInterval& i );
    void delInterval( const YInterval& i );
    bool contains( const YCursor pos ) const;

    bool isEmpty() const;

    void clear();

    YSelection clip( const YInterval& bound ) const;

    QRect boundingRect() const;

    /**
     * operators
     */
    /* shift the entier selection */
    const YSelection operator-( const YCursor pos ) const;

    static YSelection diff( const YSelection& _m1, const YSelection& _m2 );

private:
    void insertInterval( unsigned int pos, const YInterval& interval );
    void removeInterval( unsigned int pos, unsigned int len );
    int locatePosition( const YBound& pos, bool* isSelected ) const;

    QString mName;
    YSelectionMap mMap;
};

/**
 * Holds two @ref YSelection "YSelections", that is, an interval
 */
class YZIS_EXPORT YDoubleSelection
{

    friend YDebugStream& operator<<( YDebugStream& out, const YDoubleSelection& s );

public:
    YDoubleSelection( const QString& name );
    virtual ~YDoubleSelection();

    YSelectionMap screenMap() const;
    YSelectionMap bufferMap() const;
    inline const YSelection& screen() const
    {
        return *sSelection;
    }
    inline const YSelection& buffer() const
    {
        return *bSelection;
    }

    void addInterval( const YInterval& bi, const YInterval& si );
    void delInterval( const YInterval& bi, const YInterval& si );

    bool contains( const YCursor pos ) const;

    bool isEmpty() const;
    void clear();
private:
    YSelection* bSelection;
    YSelection* sSelection;
};

/**
 * Holds a @ref YSelection and a @ref YDoubleSelection
 */
class YZIS_EXPORT YSelectionPool
{
public:
    enum SelectionLayout {
        None = 0x0,
        Visual = 0x1,
        Search = 0x2,
    };

    YSelectionPool();
    virtual ~YSelectionPool();

    bool isSelected( const YCursor pos ) const;

    void setSearch( YSelection* s );

    YSelection* search();
    YDoubleSelection* visual();

private:
    YSelection* mSearch;
    YDoubleSelection* mVisual;
};

#endif

