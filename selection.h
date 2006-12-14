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

class YZIS_EXPORT YZBound {
	public:
		YZBound( const YZBound& bound ) : mPos( bound.pos() ) {
			mOpen = bound.opened();
		}
		YZBound( const YZCursor& pos, bool open = false ) : mPos( pos ) {
			mOpen = open;
		}
		YZBound(){}

		void setPos( const YZCursor& pos );
		void setPos( unsigned int x, unsigned int y );
		void open();
		void close();
		const YZCursor& pos() const;
		bool opened() const;
		bool closed() const;

	private:
		YZCursor mPos;
		bool mOpen;
};

bool operator==( const YZBound& left, const YZBound& right );
bool operator>( const YZBound& left, const YZBound& right );
bool operator<( const YZBound& left, const YZBound& right );
bool operator>=( const YZBound& left, const YZBound& right );
bool operator<=( const YZBound& left, const YZBound& right );
bool operator>=( const YZBound& left, const YZCursor& right );
bool operator<=( const YZBound& left, const YZCursor& right );
bool operator>=( const YZCursor& left, const YZBound& right );
bool operator<=( const YZCursor& left, const YZBound& right );
const YZBound operator-( const YZBound& left, const YZCursor& right );

class YZIS_EXPORT YZInterval {

	friend YZDebugStream& operator<<( YZDebugStream& out, const YZInterval& i );

	public:
		YZInterval( const YZBound& from, const YZBound& to ) : mFrom( from ), mTo( to ) {}
		YZInterval( const YZCursor& from, const YZCursor& to ) : mFrom( from ), mTo( to ) {}
		YZInterval( const YZBound& from, const YZCursor& to ) : mFrom( from ), mTo( to ) {}
		YZInterval( const YZCursor& from, const YZBound& to ) : mFrom( from ), mTo( to ) {}
		YZInterval( const QRect& r ) : mFrom( YZCursor(r.left(),r.top()) ), mTo( YZCursor(r.right(),r.bottom()) ) {}
		YZInterval(){}

		void setFrom( const YZBound& bound );
		void setTo( const YZBound& bound );
		const YZBound& from() const;
		const YZBound& to() const;

		void setFromPos( const YZCursor& pos );
		void setToPos( const YZCursor& pos );
		const YZCursor& fromPos() const;
		const YZCursor& toPos() const;

		bool contains( const YZCursor& pos ) const;
		bool contains( const YZInterval& pos ) const;

		QRect boundingRect() const;

	private:
		YZBound mFrom;
		YZBound mTo;
};
YZIS_EXPORT const YZInterval operator-( const YZInterval& l, const YZCursor& r );

typedef QMap<unsigned int, YZInterval> YZSelectionMap;

class YZIS_EXPORT YZSelection {

	friend YZIS_EXPORT YZDebugStream& operator<<( YZDebugStream& out, const YZSelection& s );

	public:
		YZSelection();
		YZSelection( const QString& name );
		YZSelection( const YZInterval& i );

		YZSelectionMap map() const;
		void setMap( const YZSelectionMap& m );

		void addMap( const YZSelectionMap& m );
		void addInterval( const YZInterval& i );
		void delInterval( const YZInterval& i );
		bool contains( const YZCursor& pos ) const;

		bool isEmpty() const;

		void clear();

		YZSelection clip( const YZInterval& bound ) const;

		QRect boundingRect() const;
		
		/** 
		 * operators
		 */
		/* shift the entier selection */
		const YZSelection operator-( const YZCursor& pos ) const;

		static YZSelection diff( const YZSelection& _m1, const YZSelection& _m2 );

	private:
		void insertInterval( unsigned int pos, const YZInterval& interval );
		void removeInterval( unsigned int pos, unsigned int len );
		int locatePosition( const YZBound& pos, bool* isSelected ) const;
		
		QString mName;
		YZSelectionMap mMap;
};

class YZIS_EXPORT YZDoubleSelection {

	friend YZDebugStream& operator<<( YZDebugStream& out, const YZDoubleSelection& s );

	public:
		YZDoubleSelection( const QString& name );
		virtual ~YZDoubleSelection();

		YZSelectionMap screenMap() const;
		YZSelectionMap bufferMap() const;
		inline const YZSelection& screen() const {
			return *sSelection;
		}
		inline const YZSelection& buffer() const {
			return *bSelection;
		}

		void addInterval( const YZInterval& bi, const YZInterval& si );
		void delInterval( const YZInterval& bi, const YZInterval& si );

		bool contains( const YZCursor& pos ) const;

		bool isEmpty() const;
		void clear();
	private:
		YZSelection* bSelection;
		YZSelection* sSelection;
};

class YZIS_EXPORT YZSelectionPool {
	public:
		enum Layout_enum {
			None	= 0x0,
			Visual	= 0x1,
			Search	= 0x2,
		};

		YZSelectionPool();
		virtual ~YZSelectionPool();

		bool isSelected( const YZCursor& pos ) const;

		void setSearch( YZSelection* s );

		YZSelection* search();
		YZDoubleSelection* visual();

	private:
		YZSelection* mSearch;
		YZDoubleSelection* mVisual;
};

#endif

