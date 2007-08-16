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

#include "font.h"

#define dbg()    yzDebug("YFont")
#define err()    yzError("YFont")

YFont::YFont()
{}
YFont::~YFont()
{}

void YFont::setWeight( int weight )
{
    m_weight = weight;
}
void YFont::setItalic( bool enable )
{
    m_italic = enable;
}
void YFont::setUnderline( bool enable )
{
    m_underline = enable;
}
void YFont::setOverline( bool enable )
{
    m_overline = enable;
}
void YFont::setStrikeOut( bool enable )
{
    m_strikeOut = enable;
}

bool YFont::bold() const
{
    return m_weight > Normal;
}
int YFont::weight() const
{
    return m_weight;
}
bool YFont::italic() const
{
    return m_italic;
}
bool YFont::underline() const
{
    return m_underline;
}
bool YFont::overline() const
{
    return m_overline;
}
bool YFont::strikeOut() const
{
    return m_strikeOut;
}

