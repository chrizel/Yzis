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

#include "selection.h"

#include "debug.h"
#include "yzis.h"

/**
 * YBound
 */

#define dbg()    yzDebug("YBound")
#define err()    yzError("YBound")

void YBound::open()
{
    mOpen = true;
}
void YBound::close()
{
    mOpen = false;
}
const YCursor YBound::pos() const
{
    return mPos;
}
bool YBound::opened() const
{
    return mOpen;
}
bool YBound::closed() const
{
    return !mOpen;
}
// operators on bounds
bool operator==( const YBound& left, const YBound& right )
{
    return left.pos() == right.pos() && left.opened() == right.opened();
}
bool operator>( const YBound& left, const YBound& right )
{
    return left.pos() > right.pos() || (left.pos() == right.pos() && !left.opened() && right.opened());
}
bool operator<( const YBound& left, const YBound& right )
{
    return left.pos() < right.pos() || (left.pos() == right.pos() && left.opened() && !right.opened());
}
bool operator>=( const YBound& left, const YBound& right )
{
    return left.pos() > right.pos() || (left.pos() == right.pos() && ( !left.opened() || right.opened() ));
}
bool operator<=( const YBound& left, const YBound& right )
{
    return left.pos() < right.pos() || (left.pos() == right.pos() && ( left.opened() || !right.opened() ));
}
bool operator>=( const YBound& left, const YCursor right )
{
    return left.pos() > right || (!left.opened() && left.pos() == right);
}
bool operator<=( const YBound& left, const YCursor right )
{
    return left.pos() < right || (!left.opened() && left.pos() == right);
}
bool operator>=( const YCursor left, const YBound& right )
{
    return right <= left;
}
bool operator<=( const YCursor left, const YBound& right )
{
    return right >= left;
}

const YBound operator-( const YBound& left, const YCursor right )
{
    return YBound( (QPoint)left.pos() - right, left.opened() );
}
/**
 * YInterval
 */

void YInterval::setFrom( const YBound& bound )
{
    mFrom = bound;
}
void YInterval::setTo( const YBound& bound )
{
    mTo = bound;
}
void YInterval::setFromPos( const YCursor pos )
{
    mFrom.setPos( pos );
}
void YInterval::setToPos( const YCursor pos )
{
    mTo.setPos( pos );
}
const YBound& YInterval::from() const
{
    return mFrom;
}
const YBound& YInterval::to() const
{
    return mTo;
}
const YCursor YInterval::fromPos() const
{
    return mFrom.pos();
}
const YCursor YInterval::toPos() const
{
    return mTo.pos();
}

bool YInterval::contains( const YBound& pos ) const 
{
	return mFrom >= pos && pos <= mTo;
}
bool YInterval::contains( const YCursor pos ) const
{
    return mFrom >= pos && pos <= mTo;
}
bool YInterval::contains( const YInterval& i ) const
{
    return mFrom <= i.from() && mTo >= i.to();
}
bool YInterval::overlap( const YInterval& i ) const
{
	return contains(i.from()) || contains(i.to()) || i.contains(from()) || i.contains(to());
}
YInterval YInterval::intersection( const YInterval& i ) const
{
	return YInterval(i.from() < from() ? from() : i.from(), i.to() < to() ? i.to() : to());
}
bool YInterval::valid() const
{
	return from() <= to();
}

QRect YInterval::boundingRect() const
{
    QRect r;
    if ( mFrom.pos().x() <= mTo.pos().x() ) {
        r.setLeft( mFrom.pos().x() );
        r.setRight( mTo.pos().x() );
    } else {
        r.setLeft( mTo.pos().x() );
        r.setRight( mFrom.pos().x() );
    }
    r.setTop( mFrom.pos().y() );
    r.setBottom( mTo.pos().y() );
    return r;
}

const YInterval operator- ( const YInterval& l, const YCursor r )
{
    return YInterval( qMax(l.from() - r, YBound(YCursor(0, 0))), qMax(l.to() - r, YBound(YCursor(0, 0), true)) );
}

QString YInterval::toString() const {
	QString out;
    if ( from().opened() )
        out += from().pos().toString() + "]";
    else
        out += "[" + from().pos().toString();
    out += "<==============>";
    if ( to().opened() )
        out += "[" + to().pos().toString();
    else
		out += to().pos().toString() + "]";
    return out;
}

YDebugStream& operator<<( YDebugStream& out, const YInterval& i )
{
	out << i.toString();
	return out;
}


/**
 * YSelection
 */
YSelection::YSelection()
{
    mMap.clear();
}
YSelection::YSelection( const QString& name )
{
    mName = name;
    mMap.clear();
}
YSelection::YSelection( const YInterval& i )
{
    addInterval( i );
}
void YSelection::setMap( const YSelectionMap& m )
{
    clear();
    mMap = m;
}
YSelectionMap YSelection::map() const
{
    return mMap;
}
bool YSelection::isEmpty() const
{
    return mMap.isEmpty();
}

void YSelection::addMap( const YSelectionMap& m )
{
    for ( int i = 0; i < m.size(); i++ )
        addInterval( m[ i ] );
}
void YSelection::addInterval( const YInterval& i )
{
    bool containsFrom;
    bool containsTo;
    unsigned int idFrom = locatePosition( i.from(), &containsFrom );
    unsigned int idTo = locatePosition( i.to(), &containsTo );
    if ( containsFrom && containsTo ) {
        if ( idFrom != idTo ) {
            mMap[ idFrom ].setTo( mMap[ idTo ].to() );
            removeInterval( idFrom + 1, idTo - idFrom );
        }
    } else if ( containsFrom ) {
        mMap[ idFrom ].setTo( i.to() );
        removeInterval( idFrom + 1, idTo - idFrom - 1 );
    } else if ( containsTo ) {
        mMap[ idTo ].setFrom( i.from() );
        removeInterval( idFrom, idTo - idFrom );
    } else if ( idTo != idFrom ) {
        mMap[ idFrom ].setFrom( i.from() );
        mMap[ idFrom ].setTo( i.to() );
        removeInterval( idFrom + 1, idTo - idFrom );
    } else {
        insertInterval( idFrom, i );
    }
}
void YSelection::delInterval( const YInterval& i )
{
    bool containsFrom;
    bool containsTo;
    unsigned int idFrom = locatePosition( i.from(), &containsFrom );
    unsigned int idTo = locatePosition( i.to(), &containsTo );
    if ( idFrom == idTo && !containsFrom && !containsTo ) return ;
    // dbg() << "delInterval: from=" << idFrom << "," << containsFrom << "; to=" << idTo << "," << containsTo << endl;

    if ( containsFrom && i.from() <= mMap[ idFrom ].from() ) {
        containsFrom = false;
    }
    if ( containsTo && i.to() >= mMap[ idTo ].to() ) {
        ++idTo;
        containsTo = false;
    }
    if ( containsTo && containsFrom && idFrom == idTo ) {
        ++idTo;
        insertInterval( idTo, mMap[ idFrom ] );
    }

    if ( containsFrom )
        mMap[ idFrom ].setTo( YBound(i.from().pos(), !i.from().opened()) );
    if ( containsTo )
        mMap[ idTo ].setFrom( YBound(i.to().pos(), !i.to().opened()) );
    removeInterval( idFrom + (containsFrom ? 1 : 0), idTo - idFrom - (containsFrom && containsTo ? 1 : 0) );
}

void YSelection::insertInterval( unsigned int pos, const YInterval& interval )
{
    unsigned int size = mMap.size() + 1;
    for ( unsigned int i = size - 1; i > pos; i-- ) {
        mMap[ i ] = mMap[ i - 1 ];
    }
    mMap.insert( pos, interval );
}
void YSelection::removeInterval( unsigned int pos, unsigned int len )
{
    if ( len == 0 ) return ;
    unsigned int size = mMap.size();
	YASSERT( size >= len );
	unsigned int i = pos;
	for ( ; i < size - len; ++i )
		mMap[ i ] = mMap[ i + len ];
	for ( ; i < size; i++ ) {
		mMap.remove( i );
	}
}

int YSelection::locatePosition( const YBound& pos, bool* isSelected ) const
{
    unsigned int i;
    *isSelected = false;
    unsigned int size = mMap.size( );
    for ( i = 0; ! *isSelected && i < size; i++ ) {
        if ( mMap[ i ].from() > pos ) break;
        if ( mMap[ i ].to() < pos ) continue;
        *isSelected = true;
    }
    if ( *isSelected ) --i;
    return i;
}

bool YSelection::contains( const YCursor pos ) const
{
    bool ret = false;
    locatePosition( YBound(pos), &ret );
    return ret;
}
void YSelection::clear()
{
    mMap.clear();
}

YSelection YSelection::clip( const YInterval& bound ) const
{

    // dbg() << "YSelection::clip " << bound << endl << "*** INPUT ***" << endl << *this << "*** END INPUT ***" << endl;

    YBound limitFrom( bound.fromPos(), !bound.from().opened() );
    YBound limitTo( bound.toPos(), !bound.to().opened() );

    YSelection tmp( mName );
    tmp.setMap( mMap );
    YBound lastBound = mMap[ mMap.size() - 1 ].to();
    YBound firstBound = mMap[ 0 ].from();
    if ( lastBound > limitTo )
        tmp.delInterval( YInterval( limitTo, lastBound ) );
    if ( !tmp.isEmpty() && firstBound < limitFrom )
        tmp.delInterval( YInterval( firstBound, limitFrom ) );

    // dbg() << "*** TMP ****" << endl << tmp << "*** END TMP ***" << endl;

    YSelection ret( mName );

    unsigned int startX = bound.fromPos().x();
    unsigned int endX = bound.toPos().x();
    unsigned int fX, tX, fY, tY;
    for ( int i = 0; i < tmp.mMap.size(); ++i ) {
        fX = tmp.mMap[ i ].fromPos().x();
        fY = tmp.mMap[ i ].fromPos().y();
        tX = tmp.mMap[ i ].toPos().x();
        tY = tmp.mMap[ i ].toPos().y();
        if ( !( fY == tY && ( fX > endX || tX < startX ) ) ) {
            if ( fX <= endX ) {
                fX = qMax( fX, startX );
            } else {
                ++fY;
                fX = startX;
            }
            if ( tX >= startX ) {
                tX = qMin( tX, endX );
            } else if ( fY < tY ) {
                --tY;
                tX = endX;
            } else {
                continue;
            }
            ret.addInterval( YInterval( YCursor( fX, fY ), YCursor( tX, tY ) ) );
        }
    }

    // dbg() << "#### result #####" << endl << ret << "#### end result ####" << endl;

    return ret;
}

YSelection YSelection::diff( const YSelection& _m1, const YSelection& _m2 )
{
    YSelection ret( _m1.mName );
    unsigned int i;
    YInterval iv;
    YBound b1, b2, n;
    bool cb1, cb2;
    int ib1, ib2;

    YSelection m1 = _m1;
    YSelection m2 = _m2;
    for ( int k = 0; k <= 1; k++ ) {
        YSelectionMap s1 = m1.map();
        YSelectionMap s2 = m2.map();
        unsigned int size1 = s1.size();
        //  unsigned int size2 = s2.size();
        for ( i = 0; i < size1; i++ ) {
            b1 = s1[ i ].from();
            b2 = s1[ i ].to();
            ib1 = m2.locatePosition( b1, &cb1 );
            ib2 = m2.locatePosition( b2, &cb2 );
            if ( cb1 && cb2 && ib1 == ib2 ) {
                continue;
            } else {
                if ( cb1 ) {
                    b1 = s2[ ib1 ].to();
                    if ( b1.closed() ) {
                        b1.open();
                    } else {
                        b1.close();
                    }
                }
                if ( cb2 ) {
                    b2 = s2[ ib2 ].from();
                    if ( b1.opened() ) {
                        b2.close();
                    } else {
                        b2.open();
                    }
                }
                for ( int j = ib1 + 1; j < ib2; j++ ) {
                    n = s2[ j ].from();
                    if ( n.opened() )
                        n.close();
                    else
                        n.open();
                    iv.setFrom( b1 );
                    iv.setTo( n );
                    ret.addInterval( iv );
                    b1 = s2[ j ].to();
                    if ( b1.opened() )
                        b1.close();
                    else
                        b1.open();
                }
                iv.setFrom( b1 );
                iv.setTo( b2 );
                ret.addInterval( iv );
            }
        }
        m1 = _m2;
        m2 = _m1;
    }

    // dbg() << "YSelection::diff: " << endl << _m1 << endl << _m2 << endl << " ====> " << ret << endl;

    return ret;
}

QRect YSelection::boundingRect() const
{
    QRect r;
    for ( int i = 0; i < mMap.size(); ++i ) {
        r |= mMap[i].boundingRect();
    }
    return r;
}

// operators on selections
YDebugStream& operator<<( YDebugStream& out, const YSelection& s )
{
    unsigned int size = s.mMap.size();
    for ( unsigned int i = 0; i < size; i++ )
        out << "(" << s.mName << " " << i << ") " << s.mMap[ i ] << endl;
    return out;
}
const YSelection YSelection::operator-( const YCursor pos ) const
{
    YSelection ret( mName );
    int i;
    int size = mMap.size();
    for ( i = 0; i < size && mMap[i].to() < YBound(pos); ++i )
        ;
    for ( ; i < size; ++i ) {
        ret.addInterval( mMap[i] - pos );
    }
    return ret;
}


