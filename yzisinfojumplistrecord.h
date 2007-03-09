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
 * Class YZYzisinfoJumpListRecord
 * 
 * Class to hold the jump positions which is used within and between files
 */

class YZYzisinfoJumpListRecord {
	public:
		/**
		 * Constructor
		 */
		 
		YZYzisinfoJumpListRecord();
		
		/**
		 * Constructor
		 * 
		 * @param filename The filename of the current file
		 * @param x The x coordinate cursor position in the current file
		 * @param y The y coordinate cursor position in the current file
		 */
		 
		YZYzisinfoJumpListRecord( const QString & filename, const unsigned int x, const unsigned int y );
		
		/**
		 * Destructor
		 */
		 
		~YZYzisinfoJumpListRecord();
		
		/**
		 * Copy constructor
		 * 
		 * @param copy The YZYzisinfoJumpListRecord instance to copy
		 */
		 
		YZYzisinfoJumpListRecord( const YZYzisinfoJumpListRecord & copy );
		
		/**
		 * Assignment operator
		 * 
		 * @param copy The YZYzisinfoJumpListRecord instance to assign from
		 */
		 
		YZYzisinfoJumpListRecord & operator=( const YZYzisinfoJumpListRecord & copy );
		
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
		 
		YZCursor  position();
		const YZCursor  position() const;
		
		/**
		 * Returns the previous recorded search cursor position
		 */
		 
		//YZCursor * previousSearchPosition();
		
		/**
		 * Sets the current filename
		 * 
		 * @param filename The current filename
		 */
		 
		void setFilename( const QString & filename );
		
		/**
		 * Sets the cursor position to record
		 * 
		 * @param x The x coordinate cursor position
		 * @param y The y coordinate cursor position
		 */
		 
		void setPosition( const unsigned int x, const unsigned int y );

	private:
		QString mFilename;
		YZCursor mPosition;
};

#endif // YZISINFOJUMPLISTRECORD_H

/*
 * END OF HEADER
 */
