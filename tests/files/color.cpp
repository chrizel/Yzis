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

#include "color.h"

/*
 * Most of the code here has been copy/pasted from the Qt3 QColor class.
 * If some things looks weird, complain to Trolltech.
 */


/** helper functions from qt ( gui/painting/qcolor_p.cpp ) **/
static int hex2int(QChar hexchar)
{
    int v;
    if (hexchar.isDigit())
        v = hexchar.digitValue();
    else if (hexchar >= 'A' && hexchar <= 'F')
        v = hexchar.cell() - 'A' + 10;
    else if (hexchar >= 'a' && hexchar <= 'f')
        v = hexchar.cell() - 'a' + 10;
    else
        v = -1;
    return v;
}

#define qRgb YzqRgb

static const struct RGBData
{
    const char *name;
    uint value;
}
rgbTbl[] = {
               { "aliceblue", qRgb(240, 248, 255) },
               { "antiquewhite", qRgb(250, 235, 215) },
               { "aqua", qRgb( 0, 255, 255) },
               { "aquamarine", qRgb(127, 255, 212) },
               { "azure", qRgb(240, 255, 255) },
               { "beige", qRgb(245, 245, 220) },
               { "bisque", qRgb(255, 228, 196) },
               { "black", qRgb( 0, 0, 0) },
               { "blanchedalmond", qRgb(255, 235, 205) },
               { "blue", qRgb( 0, 0, 255) },
               { "blueviolet", qRgb(138, 43, 226) },
               { "brown", qRgb(165, 42, 42) },
               { "burlywood", qRgb(222, 184, 135) },
               { "cadetblue", qRgb( 95, 158, 160) },
               { "chartreuse", qRgb(127, 255, 0) },
               { "chocolate", qRgb(210, 105, 30) },
               { "coral", qRgb(255, 127, 80) },
               { "cornflowerblue", qRgb(100, 149, 237) },
               { "cornsilk", qRgb(255, 248, 220) },
               { "crimson", qRgb(220, 20, 60) },
               { "cyan", qRgb( 0, 255, 255) },
               { "darkblue", qRgb( 0, 0, 139) },
               { "darkcyan", qRgb( 0, 139, 139) },
               { "darkgoldenrod", qRgb(184, 134, 11) },
               { "darkgray", qRgb(169, 169, 169) },
               { "darkgreen", qRgb( 0, 100, 0) },
               { "darkgrey", qRgb(169, 169, 169) },
               { "darkkhaki", qRgb(189, 183, 107) },
               { "darkmagenta", qRgb(139, 0, 139) },
               { "darkolivegreen", qRgb( 85, 107, 47) },
               { "darkorange", qRgb(255, 140, 0) },
               { "darkorchid", qRgb(153, 50, 204) },
               { "darkred", qRgb(139, 0, 0) },
               { "darksalmon", qRgb(233, 150, 122) },
               { "darkseagreen", qRgb(143, 188, 143) },
               { "darkslateblue", qRgb( 72, 61, 139) },
               { "darkslategray", qRgb( 47, 79, 79) },
               { "darkslategrey", qRgb( 47, 79, 79) },
               { "darkturquoise", qRgb( 0, 206, 209) },
               { "darkviolet", qRgb(148, 0, 211) },
               { "deeppink", qRgb(255, 20, 147) },
               { "deepskyblue", qRgb( 0, 191, 255) },
               { "dimgray", qRgb(105, 105, 105) },
               { "dimgrey", qRgb(105, 105, 105) },
               { "dodgerblue", qRgb( 30, 144, 255) },
               { "firebrick", qRgb(178, 34, 34) },
               { "floralwhite", qRgb(255, 250, 240) },
               { "forestgreen", qRgb( 34, 139, 34) },
               { "fuchsia", qRgb(255, 0, 255) },
               { "gainsboro", qRgb(220, 220, 220) },
               { "ghostwhite", qRgb(248, 248, 255) },
               { "gold", qRgb(255, 215, 0) },
               { "goldenrod", qRgb(218, 165, 32) },
               { "gray", qRgb(128, 128, 128) },
               { "green", qRgb( 0, 128, 0) },
               { "greenyellow", qRgb(173, 255, 47) },
               { "grey", qRgb(128, 128, 128) },
               { "honeydew", qRgb(240, 255, 240) },
               { "hotpink", qRgb(255, 105, 180) },
               { "indianred", qRgb(205, 92, 92) },
               { "indigo", qRgb( 75, 0, 130) },
               { "ivory", qRgb(255, 255, 240) },
               { "khaki", qRgb(240, 230, 140) },
               { "lavender", qRgb(230, 230, 250) },
               { "lavenderblush", qRgb(255, 240, 245) },
               { "lawngreen", qRgb(124, 252, 0) },
               { "lemonchiffon", qRgb(255, 250, 205) },
               { "lightblue", qRgb(173, 216, 230) },
               { "lightcoral", qRgb(240, 128, 128) },
               { "lightcyan", qRgb(224, 255, 255) },
               { "lightgoldenrodyellow", qRgb(250, 250, 210) },
               { "lightgray", qRgb(211, 211, 211) },
               { "lightgreen", qRgb(144, 238, 144) },
               { "lightgrey", qRgb(211, 211, 211) },
               { "lightpink", qRgb(255, 182, 193) },
               { "lightsalmon", qRgb(255, 160, 122) },
               { "lightseagreen", qRgb( 32, 178, 170) },
               { "lightskyblue", qRgb(135, 206, 250) },
               { "lightslategray", qRgb(119, 136, 153) },
               { "lightslategrey", qRgb(119, 136, 153) },
               { "lightsteelblue", qRgb(176, 196, 222) },
               { "lightyellow", qRgb(255, 255, 224) },
               { "lime", qRgb( 0, 255, 0) },
               { "limegreen", qRgb( 50, 205, 50) },
               { "linen", qRgb(250, 240, 230) },
               { "magenta", qRgb(255, 0, 255) },
               { "maroon", qRgb(128, 0, 0) },
               { "mediumaquamarine", qRgb(102, 205, 170) },
               { "mediumblue", qRgb( 0, 0, 205) },
               { "mediumorchid", qRgb(186, 85, 211) },
               { "mediumpurple", qRgb(147, 112, 219) },
               { "mediumseagreen", qRgb( 60, 179, 113) },
               { "mediumslateblue", qRgb(123, 104, 238) },
               { "mediumspringgreen", qRgb( 0, 250, 154) },
               { "mediumturquoise", qRgb( 72, 209, 204) },
               { "mediumvioletred", qRgb(199, 21, 133) },
               { "midnightblue", qRgb( 25, 25, 112) },
               { "mintcream", qRgb(245, 255, 250) },
               { "mistyrose", qRgb(255, 228, 225) },
               { "moccasin", qRgb(255, 228, 181) },
               { "navajowhite", qRgb(255, 222, 173) },
               { "navy", qRgb( 0, 0, 128) },
               { "oldlace", qRgb(253, 245, 230) },
               { "olive", qRgb(128, 128, 0) },
               { "olivedrab", qRgb(107, 142, 35) },
               { "orange", qRgb(255, 165, 0) },
               { "orangered", qRgb(255, 69, 0) },
               { "orchid", qRgb(218, 112, 214) },
               { "palegoldenrod", qRgb(238, 232, 170) },
               { "palegreen", qRgb(152, 251, 152) },
               { "paleturquoise", qRgb(175, 238, 238) },
               { "palevioletred", qRgb(219, 112, 147) },
               { "papayawhip", qRgb(255, 239, 213) },
               { "peachpuff", qRgb(255, 218, 185) },
               { "peru", qRgb(205, 133, 63) },
               { "pink", qRgb(255, 192, 203) },
               { "plum", qRgb(221, 160, 221) },
               { "powderblue", qRgb(176, 224, 230) },
               { "purple", qRgb(128, 0, 128) },
               { "red", qRgb(255, 0, 0) },
               { "rosybrown", qRgb(188, 143, 143) },
               { "royalblue", qRgb( 65, 105, 225) },
               { "saddlebrown", qRgb(139, 69, 19) },
               { "salmon", qRgb(250, 128, 114) },
               { "sandybrown", qRgb(244, 164, 96) },
               { "seagreen", qRgb( 46, 139, 87) },
               { "seashell", qRgb(255, 245, 238) },
               { "sienna", qRgb(160, 82, 45) },
               { "silver", qRgb(192, 192, 192) },
               { "skyblue", qRgb(135, 206, 235) },
               { "slateblue", qRgb(106, 90, 205) },
               { "slategray", qRgb(112, 128, 144) },
               { "slategrey", qRgb(112, 128, 144) },
               { "snow", qRgb(255, 250, 250) },
               { "springgreen", qRgb( 0, 255, 127) },
               { "steelblue", qRgb( 70, 130, 180) },
               { "tan", qRgb(210, 180, 140) },
               { "teal", qRgb( 0, 128, 128) },
               { "thistle", qRgb(216, 191, 216) },
               { "tomato", qRgb(255, 99, 71) },
               { "transparent", 0 },
               { "turquoise", qRgb( 64, 224, 208) },
               { "violet", qRgb(238, 130, 238) },
               { "wheat", qRgb(245, 222, 179) },
               { "white", qRgb(255, 255, 255) },
               { "whitesmoke", qRgb(245, 245, 245) },
               { "yellow", qRgb(255, 255, 0) },
               { "yellowgreen", qRgb(154, 205, 50) }
           };

static const int rgbTblSize = sizeof(rgbTbl) / sizeof(RGBData);

#ifdef Q_OS_TEMP
static int __cdecl rgb_cmp(const void *d1, const void *d2)
#else
static int rgb_cmp(const void *d1, const void *d2)
#endif
{
    return qstricmp(((RGBData *)d1)->name, ((RGBData *)d2)->name);
}


YZColor::YZColor()
{
    invalidate();
}

YZColor::YZColor( QRgb rgb )
{
    setRgb( rgb );
}

YZColor::YZColor( Qt::GlobalColor color )
{

    static const QRgb global_colors[] = {
                                            qRgb(255, 255, 255),  // Qt::color0
                                            qRgb( 0, 0, 0),  // Qt::color1
                                            qRgb( 0, 0, 0),  // black
                                            qRgb(255, 255, 255),  // white
                                            qRgb(128, 128, 128),  // index 248   medium gray
                                            qRgb(160, 160, 164),  // index 247   light gray
                                            qRgb(192, 192, 192),  // index 7     light gray
                                            qRgb(255, 0, 0),  // index 249   red
                                            qRgb( 0, 255, 0),  // index 250   green
                                            qRgb( 0, 0, 255),  // index 252   blue
                                            qRgb( 0, 255, 255),  // index 254   cyan
                                            qRgb(255, 0, 255),  // index 253   magenta
                                            qRgb(255, 255, 0),  // index 251   yellow
                                            qRgb(128, 0, 0),  // index 1     dark red
                                            qRgb( 0, 128, 0),  // index 2     dark green
                                            qRgb( 0, 0, 128),  // index 4     dark blue
                                            qRgb( 0, 128, 128),  // index 6     dark cyan
                                            qRgb(128, 0, 128),  // index 5     dark magenta
                                            qRgb(128, 128, 0),  // index 3     dark yellow
                                            qRgb(0, 0, 0)    //             transparent
                                        };

    setRgb( global_colors[color] );
}
YZColor::~YZColor()
{}

void YZColor::invalidate()
{
    m_valid = false;
    m_red = m_green = m_blue = -1;
}

void YZColor::setRgb( QRgb rgb )
{
    m_red = ( ( rgb >> 16 ) & 0xff ) * 0x101;
    m_green = ( ( rgb >> 8 ) & 0xff ) * 0x101;
    m_blue = ( ( rgb ) & 0xff ) * 0x101;
    m_valid = true;
}

void YZColor::setNamedColor( const QString &name )
{
    invalidate();
    if ( !name.isEmpty() ) {
        QByteArray n = name.toLatin1();
        int len = qstrlen( n.constData() );
        if (name[0] == '#') {
            QString hex( name.mid(1) );
            --len;
            m_red = m_green = m_blue = 0;
            m_valid = true;
            if (len == 6) {
                m_red = (hex2int(hex[0]) << 4) + hex2int(hex[1]);
                m_green = (hex2int(hex[2]) << 4) + hex2int(hex[3]);
                m_blue = (hex2int(hex[4]) << 4) + hex2int(hex[5]);
            } else if (len == 3) {
                m_red = (hex2int(hex[0]) << 4) + hex2int(hex[0]);
                m_green = (hex2int(hex[1]) << 4) + hex2int(hex[1]);
                m_blue = (hex2int(hex[2]) << 4) + hex2int(hex[2]);
            } else {
                m_valid = false;
            }
            if ( m_valid ) {
                m_red |= (m_red << 8);
                m_green |= (m_green << 8);
                m_blue |= (m_blue << 8);
            }
        } else { // find a color name
            ++len;
            char *name_no_space = (char *)malloc( len );
            for (int o = 0, i = 0; i < len; i++) {
                if (n[i] != '\t' && n[i] != ' ')
                    name_no_space[o++] = n[i];
            }

            RGBData x;
            x.name = name_no_space;
            RGBData *r = (RGBData*)bsearch(&x, rgbTbl, rgbTblSize, sizeof(RGBData), rgb_cmp);
            free(name_no_space);
            if (r) {
                m_valid = true;
                setRgb( r->value );
            }
        }
    }
}

bool YZColor::isValid() const
{
    return m_valid;
}
QRgb YZColor::rgb() const
{
    return qRgb( red(), green(), blue() );
}
QString YZColor::name() const
{
    QString s;
    s.sprintf("#%02x%02x%02x", red(), green(), blue() );
    return s;
}
int YZColor::red() const
{
    return m_red >> 8;
}
int YZColor::green() const
{
    return m_green >> 8;
}
int YZColor::blue() const
{
    return m_blue >> 8;
}

bool YZColor::operator!=( const YZColor& color ) const
{
    return m_red == color.m_red && m_green == color.m_green && m_blue == color.m_blue;
}

