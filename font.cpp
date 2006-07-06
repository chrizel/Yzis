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

#include "font.h"

YZFont::YZFont() {
}
YZFont::~YZFont() {
}

void YZFont::setWeight( int weight ) {
	m_weight = weight;
}
void YZFont::setItalic( bool enable ) {
	m_italic = enable;
}
void YZFont::setUnderline( bool enable ) {
	m_underline = enable;
}
void YZFont::setOverline( bool enable ) {
	m_overline = enable;
}
void YZFont::setStrikeOut( bool enable ) {
	m_strikeOut = enable;
}

bool YZFont::bold() const {
	return m_weight > Normal;
}
int YZFont::weight() const {
	return m_weight;
}
bool YZFont::italic() const {
	return m_italic;
}
bool YZFont::underline() const {
	return m_underline;
}
bool YZFont::overline() const {
	return m_overline;
}
bool YZFont::strikeOut() const {
	return m_strikeOut;
}

