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
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
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
  * The YColor class is basically a stripped down version of QColor. We can
  * not use QColor in libyzis since it is part of QtGui.
  *
  * @short Color Handling for Yzis
  */
class YZIS_EXPORT YColor
{

public:
    /** Creates an invalid color */
    YColor();

    /** Creates valid color from a rgb triplet.
      * \sa setRgb() */
    YColor( QRgb rgb );

    YColor( Qt::GlobalColor color );

    /** Creates color from a rgb triplet as a string
      * \sa setNamedColor() */
    YColor( const QString &name )
    {
        setNamedColor( name );
    }
    virtual ~YColor();

    void setRgb( QRgb );
    /**
     * @arg name can take the form:
     * "#RGB" or "#RRGGBB"
     * 
     * #123 yields the color #112233
           *
           * If the format is not correct, the color will remain invalid.
     */
    void setNamedColor( const QString &name );

    /** Return whether the color is valid.
      *
      * Initially, the color is set as invalid. It becomes valid
      * after a successful setNamedColor() or setRgb().
      */
    bool isValid() const;

    /*
     * mark the color as not valid
     */
    void invalidate();

    QRgb rgb() const;

    /** Return a string in the form #RRGGBB */
    QString name() const;

    int red() const;
    int green() const;
    int blue() const;

    bool operator!=( const YColor& color ) const;
    bool operator==( const YColor& color ) const;
	
	YColor& operator=( const YColor& color );

private:

    // rgb
    int m_red;
    int m_green;
    int m_blue;

    bool m_valid;
};

#endif
