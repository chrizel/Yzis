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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef DRAWBUFFER_H
#define DRAWBUFFER_H

#include <QString>
#include <QVector>

#include "color.h"
#include "font.h"
#include "cursor.h"
#include "selection.h"
#include "yzismacros.h"

class YZView;

typedef QMap<YZSelectionPool::Layout_enum, YZSelection> YZSelectionLayout;

struct YZDrawCell {
	bool valid;
	int flag;
	YZFont font;
	QString c;
	YZColor bg;
	YZColor fg;
	int sel;
	YZDrawCell():
		flag( 0 ),
		font(), c(), bg(), fg() {
	}
};

typedef QVector<YZDrawCell> YZDrawSection;
typedef QVector<YZDrawSection> YZDrawLine;

class YZIS_EXPORT YZDrawBuffer {

	public:

		enum whence {
			YZ_SEEK_SET, // absolute position
		};
		
		YZDrawBuffer();
		~YZDrawBuffer();

		void setCallback( YZView* v );
		void setCallbackArgument( void* callback_arg );

		/* clear the buffer */
		void reset();

		void push( const QString& c );
		void newline( int y = -1 );
		void flush();

		void setFont( const YZFont& f );
		void setColor( const YZColor& c );
		void setBackgroundColor( const YZColor& c );
		void setSelection( int sel );

		bool seek( const YZCursor& pos, YZDrawBuffer::whence w );

		YZDrawCell at( const YZCursor& pos ) const;

		void replace( const YZInterval& interval );

		void setSelectionLayout( YZSelectionPool::Layout_enum layout, const YZSelection& selection );

	private :
		void insert_section( int pos = -1 );
		void insert_line( int pos = -1 );

		void push( const QChar& c );

		void callback( int x, int y, const YZDrawCell& cell );

		bool find( const YZCursor& pos, int* x, int* y, int* vx ) const;

		void applyPosition();

		/*
		 * copy YZColor @param c into YZColor* @param dest.
		 * Returns true if *dest has changed, false else
		 */
		static bool updateColor( YZColor* dest, const YZColor& c );

		/* buffer content */
		YZDrawLine m_content;

		/* current line */
		YZDrawSection* m_line;
		/* current cell */
		YZDrawCell* m_cell;

		/* current selection layouts */
		YZSelectionLayout m_sel;

		int v_xi; /* column of the current section */
		int v_x; /* current draw column */

		int m_x; /* current section index */
		int m_y; /* current line index == current draw line */

		bool changed;
		YZDrawCell m_cur;

		YZView* m_view;
		void* m_callback_arg;
	
	friend YZDebugStream& operator<< ( YZDebugStream& out, const YZDrawBuffer& buff );

};

YZDebugStream& operator<< ( YZDebugStream& out, const YZDrawBuffer& buff );


#endif
