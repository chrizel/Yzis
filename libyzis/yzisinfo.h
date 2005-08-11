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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#ifndef YZISINFO_H
#define YZISINFO_H

#include <qdir.h>
#include <qfile.h>

#include "cursor.h"

class YZBuffer;

/**
 * Class YZYzisinfo
 * 
 * Contains all the necessary methods to read and write the yzisinfo file. 
 * This file is the equivalent of the .viminfo file in vim
 */

class YZYzisinfo {
	public:
		/**
		 * Constructor
		 */
		 
		YZYzisinfo();
		
		/**
		 * Constructor
		 * 
		 * @param path The full path to the yzisinfo file
		 */
		 
		YZYzisinfo( const QString & path );
		
		/**
		 * Destructor
		 */
		 
		~YZYzisinfo();
		
		/**
		 * Method to read the yzisinfo file
		 */
		 
		void readYzisinfo();
		
		/**
		 * Method to update the start position for the given file
		 */
		 
		void updateStartPosition( const YZBuffer *buffer, const int x, const int y );
		
		/**
		 * Method to update the jump list for the given file
		 */

		void updateJumpList( const YZBuffer *buffer, const int x, const int y );
		
		/**
		 * Method to write the yzisinfo file
		 */
		 
		void writeYzisinfo();
	
		/**
		 * Method to save the ex history to the yzisinfo file
		 * 
		 * @param write The text stream to use to write the information to the
		 * yzisinfo file
		 */
		 
		void saveExHistory( QTextStream & write );
		
	
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
		 * Method to return the start position for the current file
		 */
		
		YZCursor * startPosition( const YZBuffer *buffer );
		
		/**
		 * Method to return the previous search position for the current file
		 */
		 
		YZCursor * searchPosition( const YZBuffer *buffer );
	
	private:
		QFile mYzisinfo;
		bool mYzisinfoInitialized; // is yzisinfo initialized
};

#endif // YZISINFO_H

/*
 * END OF HEADER
 */
