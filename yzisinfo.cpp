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

#include <iostream>
#include <qfileinfo.h>

#include "debug.h"
#include "internal_options.h"
#include "session.h"
#include "yzisinfo.h"
#include "yzisinfojumplistrecord.h"
#include "yzisinfostartpositionrecord.h"

using namespace std;

class YZYzisinfoCursor;

/**
 * Constructor
 */

YZYzisinfo::YZYzisinfo() {
	
	QString path = QDir::homeDirPath() + "/.yzis";
	mYzisinfo.setName( path + "/yzisinfo" );

}

/**
 * Constructor
 */

YZYzisinfo::YZYzisinfo( const QString & path ) {
	
	mYzisinfo.setName( path );
}

/**
 * Destructor
 */

YZYzisinfo::~YZYzisinfo() {
}

/**
 * YZYsisinfo::readYzisinfo
 */

void YZYzisinfo::readYzisinfo() {
	
	if ( YZSession::me->mYzisinfoInitialized ) {
		return;
	}

	YZSession::me->mYzisinfoInitialized = true;
	
	if ( mYzisinfo.open( IO_ReadOnly ) ) {
		QTextStream stream( &mYzisinfo );
		QString line;
		
		while ( !stream.atEnd() ) {
			line = stream.readLine(); // line of text excluding '\n'
			line = line.stripWhiteSpace();
			
			// Ignore empty lines
			
			if ( line.isEmpty() ) {
				continue;
			}
			
			// Ignore comment lines

			if ( line.startsWith("#") ) {
				continue;
			}
			
			// If we have got this far then we must have a line that is 
			// a) Not a comment
			// b) Not empty
			// So split it into a list using whitespace as the separator
			
			QStringList list = QStringList::split( QRegExp( "\\s" ), line, false );
			
			if ( list[0] == "hlsearch" ) {
				if ( list[1] == "on" ) {
					bool on = true;
					YZSession::me->getOptions()->setOptionFromString( &on, "hlsearch" );
				} else {
					bool on = false; 
					YZSession::me->getOptions()->setOptionFromString( &on, "hlsearch" );
				}
			}
			
			if ( list[0].startsWith(":") || list[0] == "command_list" ) {
				YZSession::mExHistory[ YZSession::mCurrentExItem++ ] = (list.join(" ")).remove(0, 1);
			}
			
			if ( list[0].startsWith("?") || list[0] == "search_list" ) {
				YZSession::mSearchHistory[ YZSession::mCurrentSearchItem++ ] = (list.join(" ")).remove(0, 1);
			}
			
			if ( list[0].startsWith(">") || list[0] == "start_position" ) {
				YZSession::mStartPosition.push_back( new YZYzisinfoStartPositionRecord( list[3], list[1].toInt(), list[2].toInt() ) );
			}
			
			if ( list[0].startsWith("_") || list[0] == "search_history" ) {
				YZSession::mJumpList.push_back( new YZYzisinfoJumpListRecord( list[3], list[1].toInt(), list[2].toInt() ) );
				YZSession::mCurrentJumpListItem++;
			}
		}
		
		mYzisinfo.close();
	} else {
		yzDebug() << "Unable to open file " << mYzisinfo.name() << endl;
	}
}

/**
 * YZYzisinfo::updateStartPosition
 */
 
void YZYzisinfo::updateStartPosition( const YZBuffer *buffer, const int x, const int y ) {
	bool found = false;

	for ( StartPositionVector::Iterator it = YZSession::mStartPosition.begin(); it != YZSession::mStartPosition.end(); ++it ) {
		if ( (*it)->filename() == buffer->fileName() ) {
			found = true;
			YZSession::mStartPosition.erase(it);
			YZSession::mStartPosition.push_back( new YZYzisinfoStartPositionRecord( buffer->fileName(), x, y ) );
			return;
		}
	}
	         
	if ( ! found ) {         
		YZSession::mStartPosition.push_back( new YZYzisinfoStartPositionRecord( buffer->fileName(), x, y ) );
	}
	
	return;
}

/**
 * YZYzisinfo::updateJumpList
 */
 
void YZYzisinfo::updateJumpList( const YZBuffer *buffer, const int x, const int y ) {
	bool found = false;

	for ( JumpListVector::Iterator it = YZSession::mJumpList.begin(); it != YZSession::mJumpList.end(); ++it ) {
		if ( (*it)->filename() == buffer->fileName() ) {
			if ( (*it)->position().x() == static_cast<unsigned int>(x) && (*it)->position().y() == static_cast<unsigned int>(y) ) {
				found = true;
				break;
			}
		}
	}
	         
	if ( ! found ) {         
		YZSession::mJumpList.push_back( new YZYzisinfoJumpListRecord( buffer->fileName(), x, y ) );
	}
	
	return;
}

/**
 * YZYzisinfo::writeYzisinfo
 */
 
void YZYzisinfo::writeYzisinfo() {
	if ( mYzisinfo.open( IO_WriteOnly ) ) {
		QTextStream write( &mYzisinfo );
		write.setEncoding( QTextStream::UnicodeUTF8 );
		
		// Write header
		
		write << "# This yzisinfo file was generated by Yzis " << VERSION_CHAR << "." << endl;
		write << "# You may edit it if you're careful!" << endl;
		write << endl;
		
		// Write whether hlsearch is on or off
		
		write << "# Set hlsearch on or off:" << endl;
		write << "hlsearch ";
		if ( YZSession::me->getBooleanOption( "hlsearch") ) {
			write << "on" << endl;
		} else {
			write << "off" << endl;
		}
		write << endl;
		
		write << "# Command Line History (oldest to newest):" << endl; 
		saveExHistory( write );
		write << endl;
		
		write << "# Search String History (oldest to newest):" << endl;		
		saveSearchHistory( write );
		write << endl;
		
		write << "# Position to start at when opening file (oldest to newest):" << endl;
		saveStartPosition( write );
		write << endl;
		
		write << "# Jump list (oldest to newest):" << endl;
		saveJumpList( write );
		write << endl;
		
		mYzisinfo.close();
	}
}

/**
 * YZYzisinfo::saveExHistory
 */
 
void YZYzisinfo::saveExHistory( QTextStream & write ) {

	int start = 0;
	int end = YZSession::mExHistory.count();
	
	if ( end > 50) {
		start = end - 50;
	}

	for ( int i = start; i < end; ++i ) {
		if ( ! YZSession::mExHistory[i].isEmpty() ) {
			write << ":";
			write << YZSession::mExHistory[i];
			write << endl;
		}
	}
}

/**
 * YZYzisinfo::saveSearchHistory
 */
 
void YZYzisinfo::saveSearchHistory( QTextStream & write ) {
	
	int start = 0;
	int end = YZSession::mSearchHistory.count();
	
	if ( end > 50) {
		start = end - 50;
	}

	for ( int i = start; i < end; ++i ) {
		if ( ! YZSession::mSearchHistory[i].isEmpty() ) {
			write << "?";
			write << YZSession::mSearchHistory[i];
			write << endl;
		}
	}
}

/**
 * YZYzisinfo::saveStartPosition
 */
 
void YZYzisinfo::saveStartPosition( QTextStream & write ) {

	int start = 0;
	int end = YZSession::mStartPosition.count();
	
	if ( end > 100 ) {
		start = end - 100;
	}

	for( int i = start; i < end; ++i ) {
		write << "> ";
		yzDebug() << (YZSession::mStartPosition[i])->position()->x();
		write << (YZSession::mStartPosition[i])->position()->x();
		write << " "; 
		yzDebug() << (YZSession::mStartPosition[i])->position()->y();
		write << (YZSession::mStartPosition[i])->position()->y();
		write << " ";
		yzDebug() << (YZSession::mStartPosition[i])->filename() << endl;
		write << (YZSession::mStartPosition[i])->filename() << endl;
	}
}	

/**
 * YZYsisinfo::saveJumpList
 */
 
void YZYzisinfo::saveJumpList( QTextStream & write ) {

	int start = 0;	
	int end = YZSession::mJumpList.count();
	
	if ( end > 100 ) {
		start = end - 100;
	}
	
	for( int i = start; i < end; ++i ) {
		write << "_" << " ";
		write << YZSession::mJumpList[i]->position().x();
		write << " ";
		write << YZSession::mJumpList[i]->position().y();
		write << " ";
		write << YZSession::mJumpList[i]->filename() << endl;
	}
}

/**
 * YZYzisinfo::startPosition
 */

YZCursor * YZYzisinfo::startPosition( const YZBuffer *buffer ) {

	for ( StartPositionVector::Iterator it = YZSession::mStartPosition.begin(); it != YZSession::mStartPosition.end(); ++it ) {
		if ( (*it)->filename() == buffer->fileName() ) {
			return (*it)->position();
		}
	}

	return 0;
}

/**
 * YZYzisinfo::searchPosition
 */
 
YZCursor * YZYzisinfo::searchPosition( const YZBuffer */*buffer*/) {
	
	for ( JumpListVector::Iterator it = YZSession::mJumpList.begin(); it != YZSession::mJumpList.end(); ++it ) {
		/*if ( (*it)->filename() == buffer->fileName() ) {
			return (*it)->previousSearchPosition();
		}*/
	}
            
	return new YZCursor( YZSession::me->currentView()->getBufferCursor() );
}

/*
 * END OF FILE
 */
