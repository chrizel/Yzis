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

#ifndef YZISINFOSTARTPOSITIONRECORD_H
#define YZISINFOSTARTPOSITIONRECORD_H

#include <QString>
#include "cursor.h"

/**
 * Class YInfoStartPositionRecord
 * 
 * Class that holds the details about where to position the cursor in a 
 * given file when opening it.
 */

class YInfoStartPositionRecord
{
public:
    /**
     * Constructor
     */

    YInfoStartPositionRecord();

    /**
     * Constructor
     * 
     * @param filename The filename of the current file
     * @param c The cursor for the start position in the current file 
     */

    YInfoStartPositionRecord( const QString & filename, const YCursor c );



    /**
     * Destructor
     */

    ~YInfoStartPositionRecord();

    /**
     * The copy constructor
     */

    YInfoStartPositionRecord( YInfoStartPositionRecord & copy );

    /**
     * Assignment operator
     * 
     * @param copy The YInfoStartPositionRecord instance to use for the
     *             assignment
     */

    YInfoStartPositionRecord & operator=( YInfoStartPositionRecord & copy );

    /**
     * Returns the current filename
     */

    const QString & filename() const;

    /**
     * Returns the recorded start position for the current filename
     */

    YCursor position() const;

    /**
     * Sets the current filename
     * 
     * @param filename The name of the current file
     */

    void setFilename( const QString & filename );

    /**
     * Sets the current position in the current file
     * 
     * @param c The cursor position 
     */

    void setPosition( const YCursor c)
    {
        mPosition = c ;
    }

private:
    QString mFilename;
    YCursor mPosition;
};

#endif // YZISINFOSTARTPOSITIONRECORD_H

/*
 * END OF HEADER
 */
