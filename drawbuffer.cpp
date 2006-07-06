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

#include <debug.h>

#include "drawbuffer.h"
#include "view.h"

YZDrawBuffer::YZDrawBuffer() : 
	m_content(),
	m_sel()
{
	m_view = NULL;
	m_callback_arg = NULL;
	m_line = NULL;
	m_cell = NULL;
	reset();
}
YZDrawBuffer::~YZDrawBuffer() {
}

void YZDrawBuffer::setCallback( YZView* v ) {
	m_view = v;
}
void YZDrawBuffer::setCallbackArgument( void* callback_arg ) {
	m_callback_arg = callback_arg;
}
void YZDrawBuffer::callback( int x, int y, const YZDrawCell& cell ) {
	m_view->drawCell( x, y, cell, m_callback_arg );
}

void YZDrawBuffer::reset() {
	v_xi = v_x = 0;
	m_x = m_y = -1;
	m_content.clear();
	changed = false;
}

void YZDrawBuffer::flush() {
	changed = false;

	if ( m_cell == NULL || m_cell->c.length() == 0 )
		return;
	/* call callback with the undrawed part of section */
	QString keep( m_cell->c );
	m_cell->c = m_cell->c.mid( v_x - v_xi );
	callback( v_x, m_y, *m_cell );

	/* move cursor */
	v_x += m_cell->c.length();
	m_cell->c = keep;
}

void YZDrawBuffer::setFont( const YZFont& f ) {
/*	if ( f != m_cur.font ) { */
		m_cur.font = f;
/*		changed = true;
	} */
}

bool YZDrawBuffer::updateColor( YZColor* dest, const YZColor& c ) {
	bool changed = false;
	bool was_valid = dest->isValid();
	bool is_valid = c.isValid();
	if ( was_valid != is_valid || is_valid && c.rgb() != dest->rgb() ) {
		changed = true;
		if ( is_valid ) {
			dest->setRgb( c.rgb() );
		} else {
			dest->invalidate();
		}
	}
	return changed;
}

void YZDrawBuffer::setColor( const YZColor& c ) {
	changed = updateColor( &m_cur.fg, c );
}
void YZDrawBuffer::setBackgroundColor( const YZColor& c ) {
	changed = updateColor( &m_cur.bg, c );
}
void YZDrawBuffer::setSelection( int sel ) {
	if ( sel != m_cur.sel ) {
		m_cur.sel = sel;
		changed = true;
	}
}

void YZDrawBuffer::push( const QChar& c ) {
	if ( changed ) {
		/* flush last vector */
		flush();
		/* set the new properties */
		v_xi += m_cell->c.length();
		insert_section();
	}
	m_cell->c.append( c );
}

void YZDrawBuffer::push( const QString& c ) {
	YZCursor pos( v_xi + m_cell->c.length(), m_y );
	YZCursor step(1,0);
	for ( int i = 0; i < c.length(); ++i ) {
		int sel = YZSelectionPool::None;
		foreach( YZSelectionPool::Layout_enum layout, m_sel.keys() ) {
			if ( m_sel[layout].contains(pos) )
				sel |= layout;
		}
		setSelection( sel );
		push( c[i] );
		pos = pos + step;
	}
}

void YZDrawBuffer::newline( int y ) {
	flush();
	insert_line( y );
}

void YZDrawBuffer::insert_line( int pos ) {
	if ( pos == -1 )
		pos = m_y + 1;

	if ( pos < m_content.size() )
		m_content.insert( pos, YZDrawSection() );
	else
		m_content.resize( pos + 1 );

	m_y = pos;
	m_line =& m_content[m_y];

	v_x = v_xi = 0;
	m_x = -1;
	insert_section();
}

void YZDrawBuffer::insert_section( int pos ) {
	if ( pos == -1 ) {
		pos = m_x + 1;
		if ( m_x >= 0 && m_line->at( m_x ).c.length() == 0 ) /* current section is empty, don't move */
			pos = m_x;
	}

	/* copy properties */
	YZDrawCell n;
	updateColor( &n.fg, m_cur.fg );
	updateColor( &n.bg, m_cur.bg );
	n.sel = m_cur.sel;

	if ( pos >= m_line->size() ) {
		m_line->resize( pos + 1 );
		m_line->replace( pos, n );
	} else {
		if ( m_line->at( pos ).c.length() > 0 ) {
			m_line->insert( pos, n );
		} else {
			m_line->replace( pos, n );
		}
	}

	m_x = pos;
	m_cell =& (*m_line)[m_x];
}

bool YZDrawBuffer::find( const YZCursor& pos, int* x, int* y, int* vx ) const {
	bool found = false;
	int i, cx;

	if ( (int)pos.y() < m_content.size() ) {
		const YZDrawSection& l = m_content.at(pos.y());
		cx = 0;
		for( i = 0; i < l.size(); ++i ) {
			const YZDrawCell& cell = l.at(i);
			if ( (cx + cell.c.length()) > (int)pos.x() ) {
				found = true;
				break;
			}
			cx += cell.c.length();
		}
	}

	if ( found ) {
		if ( x ) *x = i;
		if ( y ) *y = pos.y();
		if ( vx ) *vx = cx;
	}
	return found;
}

YZDrawCell YZDrawBuffer::at( const YZCursor& pos ) const {
	YZDrawCell n;
	int x,y,vx;
	if ( find( pos, &x, &y, &vx ) ) {
		n = m_content.at( y ).at( x );
		n.c = n.c.mid( pos.x() - vx, 1 );
	}
	return n;
}

bool YZDrawBuffer::seek( const YZCursor& pos, YZDrawBuffer::whence w ) {
	YZCursor rpos;
	switch( w ) {
		case YZDrawBuffer::YZ_SEEK_SET :
			rpos = pos;
			break;
	}
	if ( find( rpos, &m_x, &m_y, &v_xi ) ) {
		v_x = pos.x();
		applyPosition();
		return true;
	}
	return false;
}

void YZDrawBuffer::applyPosition() {
	m_line =& m_content[m_y];
	m_cell =& (*m_line)[m_x];
	m_cur = *m_cell;
}

void YZDrawBuffer::replace( const YZInterval& interval ) {
	flush();
//	yzDebug() << "replace " << interval << endl;
//	yzDebug() << "before replace:" << endl << (*this) << "----" << endl;
	int fx = interval.fromPos().x();
	int fy = interval.fromPos().y();
	int tx = interval.toPos().x();
	int ty = interval.toPos().y();

	if ( fy >= m_content.size() ) {
		/* interval does'nt exists */
		insert_line( fy );
		v_x = v_xi = fx;
	} else {
		int dx, dy, dvx;
		bool has_dest = find( interval.toPos(), &dx, &dy, &dvx );

		if ( !seek( interval.fromPos(), YZDrawBuffer::YZ_SEEK_SET ) ) {
		//XXX	yzDebug() << "unable to access " << interval.fromPos() << endl;
			return;
		}

		/* cut content */
		QString append = "";
		QString prepend = m_cell->c.left( fx - v_xi );
		if ( has_dest ) {
			append = m_content.at(dy).at(dx).c.mid( tx - dvx + 1 );
			m_content[dy][dx].c = append;
		}
		m_cell->c = prepend;

		if ( m_x == dx && m_y == dy ) { /* same section */
			if ( append.length() > 0 ) {
				/* save current position */
				int s_mx = m_x, s_my = m_y, s_vxi = v_xi;

				/* insert a new section */
				insert_section(m_x+1);
				m_cell->c = append;

				/* restore old position */
				m_x = s_mx; m_y = s_my; v_xi = s_vxi;
				v_x = v_xi + prepend.length();
				applyPosition();
			}
		} else {
			if ( fy == ty && !has_dest || fy < ty && (m_x+1) < m_line->size() ) {
				 /* remove all until EOL */
				m_line->remove( m_x+1, m_line->size() - (m_x+1) );
			}
			if ( fy == ty && has_dest ) {
				/* remove sections between m_x and dx */
				m_line->remove( m_x+1, (dx - m_x) - 1 ); 
			}
			if ( fy < ty ) {
				if ( has_dest ) {
					m_content[dy].remove( 0, dx );
				}
				int len = (ty - fy) - (has_dest? 1 : 0 );
				if ( (fy + len) >= m_content.size() ) {
					len = m_content.size() - (fy+1);
				}
				m_content.remove( fy+1, len );
			}
		}
	}
//	yzDebug() << "after replace:" << endl << (*this) << "----" << endl;
}

void YZDrawBuffer::setSelectionLayout( YZSelectionPool::Layout_enum layout, const YZSelection& selection ) {
	m_sel[ layout ].setMap( selection.map() );
	yzDebug() << "setSelection: " << layout << "=" << m_sel[layout] << endl;
}

YZDebugStream& operator<< ( YZDebugStream& out, const YZDrawBuffer& buff ) {
	for ( int i = 0; i < buff.m_content.size(); ++i ) {
		out << i << ": ";
		for ( int j = 0; j < buff.m_content.at(i).size(); ++j ) {
			out << "'" << buff.m_content.at(i).at(j).c << "' ";
		}
		out << endl;
	}
	return out;
}

