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

#include "yzisinfojumplistrecord.h"

#define dbg()    yzDebug("YInfoJumpListRecord")
#define err()    yzError("YInfoJumpListRecord")

/**
 * YInfoJumpListRecord::YInfoJumpListRecord
 */

YInfoJumpListRecord::YInfoJumpListRecord()
{}

/**
 * YInfoJumpListRecord::YInfoJumpListRecord
 */

YInfoJumpListRecord::YInfoJumpListRecord( const QString & filename, const QPoint pos)
{
    mFilename = filename;
    setPosition( pos );
}

/**
 * YInfoJumpListRecord::~YInfoJumpListRecord
 */

YInfoJumpListRecord::~YInfoJumpListRecord()
{}

/**
 * YInfoJumpListRecord::YInfoJumpListRecord
 */

YInfoJumpListRecord::YInfoJumpListRecord( const YInfoJumpListRecord & copy )
{
    mFilename = copy.filename();
    mPosition = YCursor( copy.position().x(), copy.position().y() );
}

/**
 * YInfoJumpListRecord::operator=
 */

YInfoJumpListRecord & YInfoJumpListRecord::operator=( const YInfoJumpListRecord & copy )
{
    mFilename = copy.filename();
    mPosition = YCursor( copy.position().x(), copy.position().y() );

    return *this;
}

/**
 * YInfoJumpListRecord::filename
 */

QString & YInfoJumpListRecord::filename()
{
    return mFilename;
}

/**
 * YInfoJumpListRecord::position
 */

YCursor YInfoJumpListRecord::position()
{
    return mPosition;
}

/**
 * YInfoJumpListRecord::filename
 */

const QString & YInfoJumpListRecord::filename() const
{
    return mFilename;
}

/**
 * YInfoJumpListRecord::position
 */

const YCursor YInfoJumpListRecord::position() const
{
    return mPosition;
}

/**
 * YInfoJumpListRecord::previousSearchPosition
 */

/*YCursor * YInfoJumpListRecord::previousSearchPosition() {
 
 --mIndex;
 
 if ( mIndex < 0 ) {
  mIndex = mCursorData.count() - 1;
 }
 
 return mCursorData[mIndex];
}*/

/**
 * YInfoJumpListRecord::setFilename
 */

void YInfoJumpListRecord::setFilename( const QString & filename )
{
    mFilename = filename;
}

/**
 * YInfoJumpListRecord::setPosition
 */

void YInfoJumpListRecord::setPosition( const QPoint pos )
{
    mPosition = YCursor(pos);
}

/*
 * END OF FILE
 */
