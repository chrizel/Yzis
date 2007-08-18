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

#ifndef YZISINFOJUMPLISTRECORD_H
#define YZISINFOJUMPLISTRECORD_H

#include <QString>

#include "cursor.h"

/**
 * Class YInfoJumpListRecord
 * 
 * Class to hold the jump positions which is used within and between files
 */

class YInfoJumpListRecord
{
public:
    /**
     * Constructor
     */

    YInfoJumpListRecord();

    /**
     * Constructor
     * 
     * @param filename The filename of the current file
     * @param pos The cursor position in the current file
     */

    YInfoJumpListRecord( const QString & filename, const QPoint pos);

    /**
     * Destructor
     */

    ~YInfoJumpListRecord();

    /**
     * Copy constructor
     * 
     * @param copy The YInfoJumpListRecord instance to copy
     */

    YInfoJumpListRecord( const YInfoJumpListRecord & copy );

    /**
     * Assignment operator
     * 
     * @param copy The YInfoJumpListRecord instance to assign from
     */

    YInfoJumpListRecord & operator=( const YInfoJumpListRecord & copy );

    /**
     * Returns the current index position in the mCursorData vector
     */

    //int indexCount() { return mCursorData.count(); }

    /**
     * Returns the current filename
     */

    QString & filename();
    const QString &filename() const;

    /**
     * Returns the recorded cursor position
     */

    YCursor position();
    const YCursor position() const;

    /**
     * Returns the previous recorded search cursor position
     */

    //YCursor * previousSearchPosition();

    /**
     * Sets the current filename
     * 
     * @param filename The current filename
     */

    void setFilename( const QString & filename );

    /**
     * Sets the cursor position to record
     * 
     * @param pos The cursor position
     */

    void setPosition( const QPoint pos);

private:
    QString mFilename;
    YCursor mPosition;
};

#endif // YZISINFOJUMPLISTRECORD_H

/*
 * END OF HEADER
 */
