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

#include <qstring.h>
#include "cursor.h"

/**
 * Class YZYzisinfoStartPositionRecord
 * 
 * Class that holds the details about where to position the cursor in a 
 * given file when opening it.
 */

class YZYzisinfoStartPositionRecord {
	public:
		/**
		 * Constructor
		 */
		 
		YZYzisinfoStartPositionRecord();
	
		/**
		 * Constructor
		 * 
		 * @param filename The filename of the current file
		 * @param x The x position of the cursor in the current file 
		 * @param y The y position of the cursor in the current file
		 */
		 
		YZYzisinfoStartPositionRecord( const QString & filename, const unsigned int x, const unsigned int y );
		
		/**
		 * Destructor
		 */
		 
		~YZYzisinfoStartPositionRecord();
		
		/**
		 * The copy constructor
		 */
		 
		YZYzisinfoStartPositionRecord( YZYzisinfoStartPositionRecord & copy );
		
		/**
		 * Assignment operator
		 * 
		 * @param copy The YZYzisinfoStartPositionRecord instance to use for the
		 *             assignment
		 */
		 
		YZYzisinfoStartPositionRecord & operator=( YZYzisinfoStartPositionRecord & copy );
	
		/**
		 * Returns the current filename
		 */ 
		 
		const QString & filename() const;
		
		/** 
		 * Returns the recorded start position for the current filename
		 */
		 
		YZCursor * position() const;
		
		/**
		 * Sets the current filename
		 * 
		 * @param filename The name of the current file
		 */
		 
		void setFilename( const QString & filename );
		
		/**
		 * Sets the current position in the current file
		 * 
		 * @param x The x coordinate of the cursor position 
		 * @param y The y coordinate of the cursor position
		 */
		 
		void setPosition( const unsigned int x, const unsigned int y );

	private:
		QString mFilename;
		YZCursor * mPosition;
};

#endif // YZISINFOSTARTPOSITIONRECORD_H

/*
 * END OF HEADER
 */
