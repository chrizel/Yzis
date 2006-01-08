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

/**
 * $Id$
 */

#ifndef DRAWBUFFER_H
#define DRAWBUFFER_H

#include <QString>
#include <QVector>

#include "font.h"
#include "color.h"

class YZFont;
class YZColor;

struct YZViewCell {
	bool isValid;
	int flag;
	YZFont font;
	QString c;
	YZColor bg;
	YZColor fg;
	YZViewCell():
		isValid( false ),
		flag( 0 ),
		font(), c(), bg(), fg() {
	}
};

typedef QVector<YZViewCell> YZViewSection;
typedef QVector<YZViewSection> YZViewLine;

class YZDrawBuffer {

	public:
		YZDrawBuffer( void(*callback)(const YZViewCell&, void*) );
		~YZDrawBuffer();

		void setCallbackArgument( void* callback_arg );
		
		void reset();
		//void seek( unsigned int x, unsigned int y );

		void push( const QString& c, bool overwrite = true );
		void flush();
		void linebreak( bool overwrite = true );
		
		void setFont( const YZFont& f );
		void setColor( const YZColor& c );
	
		//const YZViewCell& at( unsigned int x, unsigned int y );
	
	private :
		void append_section();
		void append_line();

		YZViewLine m_content;
		YZViewSection* m_line;
		YZViewCell* m_cell;

		unsigned int m_x;
		unsigned int m_xi;
		unsigned int m_y;

		bool changed;
		YZViewCell m_cur;

		void(*m_callback)(const YZViewCell&, void*);
		void* m_callback_arg;

};

#endif
