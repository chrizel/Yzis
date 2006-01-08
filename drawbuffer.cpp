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


#include "drawbuffer.h"

#include <debug.h>

YZDrawBuffer::YZDrawBuffer( void(*callback)(const YZViewCell&, void*) ) : m_content() {
	m_callback = callback;
	m_callback_arg = NULL;
	m_line = NULL;
	m_cell = NULL;
	reset();
}
YZDrawBuffer::~YZDrawBuffer() {
}

void YZDrawBuffer::setCallbackArgument( void* callback_arg ) {
	m_callback_arg = callback_arg;
}

void YZDrawBuffer::reset() {
	m_xi = m_x = m_y = 0;
	m_content.clear();
	changed = false;
	append_line();
	append_section();
}

void YZDrawBuffer::flush() {
	/* call callback with the undrawed part of section */
	QString keep( m_cell->c );
	m_cell->c = m_cell->c.mid( m_x - m_xi );
	m_callback( *m_cell, m_callback_arg );
	changed = false;

	/* move cursor */
	m_x += m_cell->c.length();
	m_cell->c = keep;
}

void YZDrawBuffer::setFont( const YZFont& f ) {
/*	if ( f != m_cur.font ) { */
		m_cur.font = f;
/*		changed = true;
	} */
}
void YZDrawBuffer::setColor( const YZColor& c ) {
	if ( c.rgb() != m_cur.fg.rgb() ) {
		m_cur.fg.setRgb( c.rgb() );
		changed = true;
	}
}

void YZDrawBuffer::push( const QString& c, bool /*overwrite*/ ) {
	if ( changed ) {
		/* flush last vector */
		flush();
		/* set the new properties */
		m_xi += m_cell->c.length();
		append_section();
	}
	m_cell->c.append( c );
}

void YZDrawBuffer::linebreak( bool overwrite ) {
	flush();
	m_y += 1;
	m_x = 0;
	m_xi = 0;
	if ( overwrite && (int)m_y < m_content.size() ) {
		m_line = &m_content[m_y];
		m_line->clear();
	} else {
		append_line();
	}
	append_section();
}

void YZDrawBuffer::append_line() {
	m_content.append( YZViewSection() );
	m_line =& m_content.last();
}
void YZDrawBuffer::append_section() {
	YZViewCell n;
	/* copy the new properties */
	n.fg.setRgb( m_cur.fg.rgb() );
	m_line->append( n );
	m_cell =& m_line->last();
}


