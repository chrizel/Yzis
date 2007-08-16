/* This file is part of the Yzis libraries
*  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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

#include "yzisinfostartpositionrecord.h"

#define dbg()    yzDebug("YInfoStartPositionRecord")
#define err()    yzError("YInfoStartPositionRecord")

/**
 * YInfoStartPositionRecord::YZYzsinfoStartPositionRecord
 */

YInfoStartPositionRecord::YInfoStartPositionRecord()
{
    mFilename = "";
    setPosition( YCursor( 0, 0 ));
}

/**
 * YInfoStartPositionRecord::YInfoStartPositionRecord
 */

YInfoStartPositionRecord::YInfoStartPositionRecord( const QString & filename, const YCursor c)
{
    mFilename = filename;
    mPosition = c;
}



/**
 * YInfoStartPositionRecord::~YZYzsinfoStartPositionRecord
 */

YInfoStartPositionRecord::~YInfoStartPositionRecord()
{}

/**
 * YInfoStartPositionRecord::YZYzsinfoStartPositionRecord
 */

YInfoStartPositionRecord::YInfoStartPositionRecord( YInfoStartPositionRecord & copy )
{
    mFilename = copy.filename();
    setPosition( copy.position() );
}

/**
 * YInfoStartPositionRecord::operator=
 */

YInfoStartPositionRecord & YInfoStartPositionRecord::operator=( YInfoStartPositionRecord & copy )
{
    if ( this == &copy ) {
        return * this;
    }

    mFilename = copy.filename();
    setPosition( copy.position() );

    return *this;
}

/**
 * YInfoStartPositionRecord::filename
 */

const QString& YInfoStartPositionRecord::filename() const
{
    return mFilename;
}

/**
 * YInfoStartPositionRecord::position
 */

YCursor YInfoStartPositionRecord::position() const
{
    return mPosition;
}

/**
 * YInfoStartPositionRecord::setFilename
 */

void YInfoStartPositionRecord::setFilename( const QString & filename )
{
    mFilename = filename;
}

/*
 * END OF FILE
 */
