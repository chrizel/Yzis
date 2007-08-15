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

#ifndef YZISINFO_H
#define YZISINFO_H

/* Qt */
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QVector>

/* Project */
#include "cursor.h"

class YZBuffer;
class YZYzisinfoStartPositionRecord;
class YZYzisinfoJumpListRecord;

typedef QVector<YZYzisinfoStartPositionRecord*> StartPositionVector;
typedef QVector<YZYzisinfoJumpListRecord*> JumpListVector;

/**
 * Class YZYzisinfo
 *
 * @short Handle the ~/.yzisinfo file.
 * 
 * Contains all the necessary methods to read and write the ~/.yzisinfo file. 
 * This file is the equivalent of the ~/.viminfo file in vim
 */

class YZYzisinfo
{
public:
    /**
     * Default Constructor.  Set the filename to
     * ~/.yzis/fileinfo, but doesn't try to read the file yet.
     */

    YZYzisinfo();

    /**
     * Constructor. Set the filename to path, but doesn't try
     * to read the file yet.
     * 
     * @param path The full path to the yzisinfo file
     */

    YZYzisinfo( const QString & path );

    /**
     * Destructor. Does nothing.
     */

    ~YZYzisinfo();

    /**
     * Actually read the yzisinfo file.
     */

    void read(void);

    /**
     * Method to update the start position for the given file
     */

    void updateStartPosition( const YZBuffer *buffer, const YZCursor cursor );

    /**
     * Method to update the jump list for the given file
     */

    void updateJumpList( const YZBuffer *buffer, const QPoint pos);

    /**
     * Method to write the yzisinfo file
     */

    void write();

    /**
     * Method to save the search history to the yzisinfo file
     * 
     * @param write The text stream to use to write the information to the 
     * yzisinfo file
     */

    void saveSearchHistory( QTextStream & write );

    /**
     * Method to save the start position to the yzisinfo file
     * 
     * @param write The text stream to use to write the information to the 
     * yzisinfo file
     */

    void saveStartPosition( QTextStream & write );

    /**
     * Method to save the jump list to the yzisinfo file
     * 
     * @param write The text stream to use to write the information to the
     * yzisinfo file
     */

    void saveJumpList( QTextStream & write );

    /**
     * Method to save the registers list to the yzisinfo file
     * 
     * @param write The text stream to use to write the information to the
     * yzisinfo file
     */

    void saveRegistersList( QTextStream & write );

    /**
     * Method to return the start position for the current file
     */

    YZCursor startPosition( const QString& filename ) const;
    YZCursor startPosition( const YZBuffer *buffer ) const;

    /**
     * Method to return the previous search position for the current file
     */

    YZCursor searchPosition( const YZBuffer *buffer );

    /**
     * Returns the previous jump position
     */
    const YZCursor previousJumpPosition();

private:
    /**
     * Method to save the ex history to the yzisinfo file
     * 
     * @param write The text stream to use to write the information to the
     * yzisinfo file
     */

    void saveExHistory( QTextStream & write );


    QFile mYzisinfo;
    bool mYzisinfoInitialized; // is yzisinfo initialized

    /**
     * start position history
     */

    StartPositionVector mStartPosition;

    /**
    * jump list history
    */

    JumpListVector mJumpList;

    /**
    * current jump list item
    */

    unsigned int mCurrentJumpListItem;
};

#endif // YZISINFO_H

/*
 * END OF HEADER
 */
