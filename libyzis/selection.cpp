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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#include "selection.h"

#include "debug.h"
#include "yzis.h"

/**
 * YZBound
 */

void YZBound::setPos( const YZCursor& pos ) {
	mPos = pos;
}
void YZBound::setPos( unsigned int x, unsigned int y ) {
	mPos.setX( x );
	mPos.setY( y );
}
void YZBound::open() {
	mOpen = true;
}
void YZBound::close() {
	mOpen = false;
}
const YZCursor& YZBound::pos() const {
	return mPos;
}
bool YZBound::opened() const {
	return mOpen;
}
bool YZBound::closed() const {
	return !mOpen;
}
// operators on bounds
bool operator==( const YZBound& left, const YZBound& right ) {
	return left.pos() == right.pos() && left.opened() == right.opened();
}
bool operator>( const YZBound& left, const YZBound& right ) {
	return left.pos() > right.pos() || left.pos() == right.pos() && !left.opened() && right.opened();
}
bool operator<( const YZBound& left, const YZBound& right ) {
	return left.pos() < right.pos() || left.pos() == right.pos() && left.opened() && !right.opened();
}
bool operator>=( const YZBound& left, const YZBound& right ) {
	return left.pos() > right.pos() || left.pos() == right.pos() && ( !left.opened() || right.opened() );
}
bool operator<=( const YZBound& left, const YZBound& right ) {
	return left.pos() < right.pos() || left.pos() == right.pos() && ( left.opened() || !right.opened() );
}
bool operator>=( const YZBound& left, const YZCursor& right ) {
	return left.pos() > right || !left.opened() && left.pos() == right;
}
bool operator<=( const YZBound& left, const YZCursor& right ) {
	return left.pos() < right || !left.opened() && left.pos() == right;
}
bool operator>=( const YZCursor& left, const YZBound& right ) {
	return right <= left;
}
bool operator<=( const YZCursor& left, const YZBound& right ) {
	return right >= left;
}

/**
 * YZInterval
 */

void YZInterval::setFrom( const YZBound& bound ) {
	mFrom = bound;
}
void YZInterval::setTo( const YZBound& bound ) {
	mTo = bound;
}
void YZInterval::setFromPos( const YZCursor& pos ) {
	mFrom.setPos( pos );
}
void YZInterval::setToPos( const YZCursor& pos ) {
	mTo.setPos( pos );
}
const YZBound& YZInterval::from() const {
	return mFrom;
}
const YZBound& YZInterval::to() const {
	return mTo;
}
const YZCursor& YZInterval::fromPos() const {
	return mFrom.pos();
}
const YZCursor& YZInterval::toPos() const {
	return mTo.pos();
}

bool YZInterval::contains( const YZCursor& pos ) const {
	return mFrom >= pos && pos <= mTo;
}
bool YZInterval::contains( const YZInterval& i ) const {
	return mFrom <= i.from() && mTo >= i.to();
}


YZDebugStream& operator<<( YZDebugStream& out, const YZInterval& i ) {
	if ( i.from().opened() )
		out << i.from().pos() << "]";
	else
		out << "[" << i.from().pos();
	out << "<==============>";
	if ( i.to().opened() )
		out << "[" << i.to().pos();
	else
		out << i.to().pos() << "]";
	return out;
}


/**
 * YZSelection
 */
YZSelection::YZSelection( const QString& name ) {
	mName = name;
	mMap.clear();
}
void YZSelection::setMap( const YZSelectionMap& m ) {
	clear();
	mMap = m;
}
YZSelectionMap YZSelection::map() const {
	return mMap;
}
bool YZSelection::isEmpty() const {
	return mMap.isEmpty();
}

void YZSelection::addMap( const YZSelectionMap& m ) {
	for ( unsigned int i = 0; i < m.size(); i++ )
		addInterval( m[ i ] );
}
void YZSelection::addInterval( const YZInterval& i ) {
	bool containsFrom;
	bool containsTo;
	unsigned int idFrom = locatePosition( i.from(), &containsFrom );
	unsigned int idTo = locatePosition( i.to(), &containsTo );
	if ( containsFrom && containsTo ) {
		if ( idFrom != idTo ) {
			mMap[ idFrom ].setTo( mMap[ idTo ].to() );
			removeInterval( idFrom+1, idTo - idFrom );
		}
	} else if ( containsFrom ) {
		mMap[ idFrom ].setTo( i.to() );
		removeInterval( idFrom+1, idTo - idFrom - 1 );
	} else if ( containsTo ) {
		mMap[ idTo ].setFrom( i.from() );
		removeInterval( idFrom, idTo - idFrom );
	} else if ( idTo != idFrom ) {
		mMap[ idFrom ].setFrom( i.from() );
		mMap[ idFrom ].setTo( i.to() );
		removeInterval( idFrom+1, idTo - idFrom );
	} else {
		insertInterval( idFrom, i );
	}
}
void YZSelection::delInterval( const YZInterval& i ) {
	bool containsFrom;
	bool containsTo;
	unsigned int idFrom = locatePosition( i.from(), &containsFrom );
	unsigned int idTo = locatePosition( i.to(), &containsTo );
	if ( idFrom == idTo && !containsFrom && !containsTo ) return;
//	yzDebug() << "delInterval: from=" << idFrom << "," << containsFrom << "; to=" << idTo << "," << containsTo << endl;

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
		mMap[ idFrom ].setTo( YZBound(i.from().pos(), !i.from().opened()) );
	if ( containsTo )
		mMap[ idTo ].setFrom( YZBound(i.to().pos(), !i.to().opened()) );
	removeInterval( idFrom+(containsFrom?1:0), idTo - idFrom - (containsFrom && containsTo ? 1 : 0) );
}

void YZSelection::insertInterval( unsigned int pos, const YZInterval& interval ) {
	unsigned int size = mMap.size() + 1;
	for ( unsigned int i = size - 1; i > pos; i-- ) {
		mMap[ i ] = mMap[ i - 1 ];
	}
	mMap.insert( pos, interval );
}
void YZSelection::removeInterval( unsigned int pos, unsigned int len ) {
	if ( len == 0 ) return;
	unsigned int i;
	unsigned int size = mMap.size();
	if ( size >= len )
		for ( i = pos; i < size - len; ++i )
			mMap[ i ] = mMap[ i + len ];
	else // should not happen
		yzDebug() << "WARNING: YZSelection::removeInterval remove more than size (" << len << " > " << size << ")" << endl;
	for ( ; i < size; i++ ) {
		mMap.remove( i );
	}
}

int YZSelection::locatePosition( const YZBound& pos, bool* isSelected ) {
	unsigned int i;
	*isSelected = false;
	unsigned int size = mMap.size( );
	for ( i = 0; ! *isSelected && i < size; i++ ) {
		if ( mMap[ i ].from() > pos ) break;
		if ( mMap[ i ].to() < pos  ) continue;
		*isSelected = true;
	}
	if ( *isSelected ) --i;
	return i;
}
bool YZSelection::contains( const YZCursor& pos ) {
	bool ret = false;
	locatePosition( pos, &ret );
	return ret;
}
void YZSelection::clear() {
	mMap.clear();
}

YZSelection YZSelection::clip( const YZInterval& bound ) const {

//	yzDebug() << "YZSelection::clip " << bound << endl << "*** INPUT ***" << endl << *this << "*** END INPUT ***" << endl;

	YZBound limitFrom( bound.fromPos(), !bound.from().opened() );
	YZBound limitTo( bound.toPos(), !bound.to().opened() );

	YZSelection tmp( mName );
	tmp.setMap( mMap );
	YZBound lastBound = mMap[ mMap.size() - 1 ].to();
	YZBound firstBound = mMap[ 0 ].from();
	if ( lastBound > limitTo )
		tmp.delInterval( YZInterval( limitTo, lastBound ) );
	if ( !tmp.isEmpty() && firstBound < limitFrom )
		tmp.delInterval( YZInterval( firstBound, limitFrom ) );

//	yzDebug() << "*** TMP ****" << endl << tmp << "*** END TMP ***" << endl;

	YZSelection ret( mName );

	unsigned int startX = bound.fromPos().x();
	unsigned int endX = bound.toPos().x();
	unsigned int fX, tX, fY, tY;
	for( unsigned int i = 0; i < tmp.mMap.size(); ++i ) {
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
			ret.addInterval( YZInterval( YZCursor( fX, fY ), YZCursor( tX, tY ) ) );
		}
	}

//	yzDebug() << "#### result #####" << endl << ret << "#### end result ####" << endl;

	return ret;
}

YZSelection YZSelection::diff( const YZSelection& _m1, const YZSelection& _m2 ) {
	YZSelection ret( _m1.mName );
	unsigned int i;
	YZInterval iv;
	YZBound b1, b2, n;
	bool cb1, cb2;
	int ib1, ib2;

	YZSelection m1 = _m1;
	YZSelection m2 = _m2;
	for ( int k = 0; k <= 1; k++ ) {
		YZSelectionMap s1 = m1.map();
		YZSelectionMap s2 = m2.map();
		unsigned int size1 = s1.size();
//		unsigned int size2 = s2.size();
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

//	yzDebug() << "YZSelection::diff : " << endl << _m1 << endl << _m2 << endl << " ====> " << ret << endl;

	return ret;
}

// operators on selections
YZDebugStream& operator<<( YZDebugStream& out, const YZSelection& s ) {
	unsigned int size = s.mMap.size();
	for ( unsigned int i = 0; i < size; i++ )
		out << "(" << s.mName << " " << i << ") " << s.mMap[ i ] << endl;
	return out;
}


YZDoubleSelection::YZDoubleSelection( const QString& name ) {
	bSelection = new YZSelection( name + " buffer" );
	sSelection = new YZSelection( name + " screen" );
}
YZDoubleSelection::~YZDoubleSelection() {
	delete bSelection;
	delete sSelection;
}
YZSelectionMap YZDoubleSelection::bufferMap() {
	return bSelection->map();
}
YZSelectionMap YZDoubleSelection::screenMap() {
	return sSelection->map();
}
bool YZDoubleSelection::isEmpty() {
	return bSelection->isEmpty();
}

void YZDoubleSelection::addInterval( const YZInterval& bi, const YZInterval& si ) {
	bSelection->addInterval( bi );
	sSelection->addInterval( si );
}
void YZDoubleSelection::delInterval( const YZInterval& bi, const YZInterval& si ) {
	bSelection->delInterval( bi );
	sSelection->delInterval( si );
}

bool YZDoubleSelection::contains( const YZCursor& pos ) {
	return bSelection->contains( pos );
}
void YZDoubleSelection::clear() {
	bSelection->clear();
	sSelection->clear();
}

YZDebugStream& operator<<( YZDebugStream& out, const YZDoubleSelection& s ) {
	out << *s.bSelection << *s.sSelection;
	return out;
}



/*
 * YZSelectionPool
 **/

YZSelectionPool::YZSelectionPool() {
	mSearch = new YZSelection( "SEARCH" );
	mVisual = new YZDoubleSelection( "VISUAL" );
}
YZSelectionPool::~YZSelectionPool( ) {
	delete mSearch;
	delete mVisual;
}
void YZSelectionPool::setSearch( YZSelection* s ) {
	mSearch->setMap( s->map() );
}
YZSelection* YZSelectionPool::search() {
	return mSearch;
}
YZDoubleSelection* YZSelectionPool::visual() {
	return mVisual;
}
bool YZSelectionPool::isSelected( const YZCursor& pos ) {
	return mSearch->contains( pos ) || mVisual->contains( pos );
}

