/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Lucijan Busch <luci@yzis.org>
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

#include <qregexp.h>
#include "line.h"
#include "syntaxdocument.h"
#include "debug.h"

YZLine::YZLine(const QString &l) :
m_flags( YZLine::flagVisible ) {
	setData(l);
	m_initialized = false;
}

YZLine::YZLine() {
	setData( "" );
	m_initialized = false;
}

YZLine::~YZLine() {
}

void YZLine::setData(const QString &data) {
	mData = data;
	uint len = data.length();
	if ( len == 0 ) len++; //make sure to return a non empty array ... (that sucks)
	mAttributes.resize( len );
	for ( uint i = 0; i < len; i++ )
		mAttributes.at(i) = 0;
}

int YZLine::firstChar() const {
	return nextNonSpaceChar(0);
}

int YZLine::lastChar() const {
	return previousNonSpaceChar(mData.length()-1);
}

int YZLine::nextNonSpaceChar(uint pos) const {
	int length = (int)mData.length();
	for (int i = pos; i < length; ++i) {
		if (!mData[i].isSpace()) 
			return i;
	}
	return -1;
}

int YZLine::previousNonSpaceChar(uint pos) const {
	if (pos >= mData.length())
		pos = mData.length() - 1;
	for (int i = pos; i>=0; --i) {
		if (!mData[i].isSpace())
			return i;
	}
	return -1;
}

