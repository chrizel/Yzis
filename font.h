/*  This file is part of the Yzis libraries
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
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

#ifndef YZ_FONT_H
#define YZ_FONT_H

class YZFont {

	public :
		enum Weight {
			Light,
			Normal,
			DemiBold,
			Bold,
			Black
		};

		YZFont();
		virtual ~YZFont();

		void setWeight( int weight );
		void setItalic( bool enable );
		void setUnderline( bool enable );
		void setOverline( bool enable );
		void setStrikeOut( bool enable );

		bool bold() const;
		int weight() const;
		bool italic() const;
		bool underline() const;
		bool overline() const;
		bool strikeOut() const;
	
	private :
		int m_weight;
		bool m_italic;
		bool m_underline;
		bool m_overline;
		bool m_strikeOut;

};

#endif
