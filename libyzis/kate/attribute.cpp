/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
*  Copyright (C) 2003-2004 Philippe Fremy <pfremy@freehackers.org>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License version 2 as published by the Free Software Foundation
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
/**
 * This file was originally taken from Kate, KDE editor
   Kate's code is published under the LGPL version 2 (and 2 only not any later 
   version)
   The copyrights follow below :
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
*/

#include "attribute.h"

#define dbg()    yzDebug("YzisAttribute")
#define err()    yzError("YzisAttribute")

YzisAttribute::YzisAttribute()
        : m_weight(YZFont::Normal)
        , m_italic(false)
        , m_underline(false)
        , m_overline(false)
        , m_strikeout(false)
        , m_itemsSet(0)

{}

YzisAttribute::~YzisAttribute()
{}

void YzisAttribute::clear()
{
    m_itemsSet = 0;
}

YzisAttribute& YzisAttribute::operator+=(const YzisAttribute& a)
{
    if (a.itemSet(Weight))
        setWeight(a.weight());

    if (a.itemSet(Italic))
        setItalic(a.italic());

    if (a.itemSet(Underline))
        setUnderline(a.underline());

    if (a.itemSet(Overline))
        setOverline(a.overline());

    if (a.itemSet(StrikeOut))
        setStrikeOut(a.strikeOut());

    if (a.itemSet(Outline))
        setOutline(a.outline());

    if (a.itemSet(TextColor))
        setTextColor(a.textColor());

    if (a.itemSet(SelectedTextColor))
        setSelectedTextColor(a.selectedTextColor());

    if (a.itemSet(BGColor))
        setBGColor(a.bgColor());

    if (a.itemSet(SelectedBGColor))
        setSelectedBGColor(a.selectedBGColor());

    return *this;
}

YZFont YzisAttribute::font(const YZFont& ref) const
{
    YZFont ret = ref;

    if (itemSet(Weight))
        ret.setWeight(weight());
    if (itemSet(Italic))
        ret.setItalic(italic());
    if (itemSet(Underline))
        ret.setUnderline(underline());
    if (itemSet(Overline))
        ret.setOverline(overline());
    if (itemSet(StrikeOut))
        ret.setStrikeOut(strikeOut());

    return ret;
}

void YzisAttribute::setWeight(int weight)
{
    if (!(m_itemsSet & Weight) || m_weight != weight) {
        m_itemsSet |= Weight;

        m_weight = weight;

        changed();
    }
}

void YzisAttribute::setBold(bool enable)
{
    setWeight(enable ? YZFont::Bold : YZFont::Normal);
}

void YzisAttribute::setItalic(bool enable)
{
    if (!(m_itemsSet & Italic) || m_italic != enable) {
        m_itemsSet |= Italic;

        m_italic = enable;

        changed();
    }
}

void YzisAttribute::setUnderline(bool enable)
{
    if (!(m_itemsSet & Underline) || m_underline != enable) {
        m_itemsSet |= Underline;

        m_underline = enable;

        changed();
    }
}

void YzisAttribute::setOverline( bool enable )
{
    if ( !( m_itemsSet & Overline ) || m_overline != enable ) {
        m_itemsSet |= Overline;

        m_overline = enable;

        changed();
    }
}

void YzisAttribute::setStrikeOut(bool enable)
{
    if (!(m_itemsSet & StrikeOut) || m_strikeout != enable) {
        m_itemsSet |= StrikeOut;

        m_strikeout = enable;

        changed();
    }
}

void YzisAttribute::setOutline(const YZColor& color)
{
    if (!(m_itemsSet & Outline) || m_outline != color) {
        m_itemsSet |= Outline;

        m_outline = color;

        changed();
    }
}

void YzisAttribute::setTextColor(const YZColor& color)
{
    if (!(m_itemsSet & TextColor) || m_textColor != color) {
        m_itemsSet |= TextColor;

        m_textColor = color;

        changed();
    }
}

void YzisAttribute::setSelectedTextColor(const YZColor& color)
{
    if (!(m_itemsSet & SelectedTextColor) || m_selectedTextColor != color) {
        m_itemsSet |= SelectedTextColor;

        m_selectedTextColor = color;

        changed();
    }
}

void YzisAttribute::setBGColor(const YZColor& color)
{
    if (!(m_itemsSet & BGColor) || m_bgColor != color) {
        m_itemsSet |= BGColor;

        m_bgColor = color;

        changed();
    }
}

void YzisAttribute::setSelectedBGColor(const YZColor& color)
{
    if (!(m_itemsSet & SelectedBGColor) || m_selectedBGColor != color) {
        m_itemsSet |= SelectedBGColor;

        m_selectedBGColor = color;

        changed();
    }
}

bool operator ==(const YzisAttribute& h1, const YzisAttribute& h2)
{
    if (h1.m_itemsSet != h2.m_itemsSet)
        return false;

    if (h1.itemSet(YzisAttribute::Weight))
        if (h1.m_weight != h2.m_weight)
            return false;

    if (h1.itemSet(YzisAttribute::Italic))
        if (h1.m_italic != h2.m_italic)
            return false;

    if (h1.itemSet(YzisAttribute::Underline))
        if (h1.m_underline != h2.m_underline)
            return false;

    if (h1.itemSet(YzisAttribute::StrikeOut))
        if (h1.m_strikeout != h2.m_strikeout)
            return false;

    if (h1.itemSet(YzisAttribute::Outline))
        if (h1.m_outline != h2.m_outline)
            return false;

    if (h1.itemSet(YzisAttribute::TextColor))
        if (h1.m_textColor != h2.m_textColor)
            return false;

    if (h1.itemSet(YzisAttribute::SelectedTextColor))
        if (h1.m_selectedTextColor != h2.m_selectedTextColor)
            return false;

    if (h1.itemSet(YzisAttribute::BGColor))
        if (h1.m_bgColor != h2.m_bgColor)
            return false;

    if (h1.itemSet(YzisAttribute::SelectedBGColor))
        if (h1.m_selectedBGColor != h2.m_selectedBGColor)
            return false;

    return true;
}

bool operator !=(const YzisAttribute& h1, const YzisAttribute& h2)
{
    return !(h1 == h2);
}

