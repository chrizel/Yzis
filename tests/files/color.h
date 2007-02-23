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

#ifndef YZ_COLOR_H
#define YZ_COLOR_H

/* Qt */
#include <QString>

/* yzis */
#include "yzismacros.h"

#ifndef QRgb
typedef unsigned int QRgb;
#endif

#define YzqRgb(r,g,b) (0xff000000 | (r << 16) |  (g << 8) | b)

/** Class to store the concept of color, for our syntax highlighting code.
  *
  * Colors are stored as triplets of red,green,blue byte values.
  *
  * The YZColor class is basically a stripped down version of QColor. We can
  * not use QColor in libyzis since it is part of QtGui.
  *
  * @short Color Handling for Yzis
  */
class YZIS_EXPORT YZColor {

	public:
		YZColor();
		YZColor( QRgb rgb );
		YZColor( Qt::GlobalColor color );
		YZColor( const QString &name ) { setNamedColor( name ); }
		virtual ~YZColor();

		void setRgb( QRgb );
		/**
		 * @arg name can take the form:
		 * "#RGB" or "#RRGGBB"
		 * 
		 * #123 yields the color #112233
		 */
		void setNamedColor( const QString &name );

		bool isValid() const;

		/*
		 * mark the color as not valid
		 */
		void invalidate();

		QRgb rgb() const;
		/* #RRGGBB */
		QString name() const;
		int red() const;
		int green() const;
		int blue() const;

		bool operator!=( const YZColor& color ) const;
	
	private:
		
		// rgb
		int m_red;
		int m_green;
		int m_blue;

		bool m_valid;

};

#endif
