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

#include "debug.h"
#include "session.h"
#include "yzisinfo.h"
#include "yzisinforecord.h"
#include "yzisinforecordstartposition.h"
#include "yzisinforecordsearchhistory.h"
#include <qfileinfo.h>

using namespace std;

class YZYzisinfoCursor;

YZYzisinfo::YZYzisinfo() {
	
	QString path = QDir::homeDirPath() + "/.yzis";
	mYzisinfo.setName( path + "/yzisinfo" );

}

YZYzisinfo::YZYzisinfo( const QString & path ) {
	
	mYzisinfo.setName( path );
}

YZYzisinfo::~YZYzisinfo() {
}

void YZYzisinfo::readYzisinfo() {
	
	if ( mYzisinfo.open( IO_ReadOnly ) ) {
		QTextStream stream( &mYzisinfo );
		QString line;
		
		while ( !stream.atEnd() ) {
			line = stream.readLine(); // line of text excluding '\n'
			line = line.stripWhiteSpace();
			
			if ( ! line.isEmpty() ) {
				QStringList list = QStringList::split( QRegExp( "\\s" ), line, false );
				if ( list[0] == "start_position" ) {
					saveStartPosition( list[3], list[1].toInt(), list[2].toInt() );
				}
				
				if ( list[0] == "search_history" ) {
					saveSearchPosition( list[3], list[1].toInt(), list[2].toInt() );
				}
			}
		}
		
		mYzisinfo.close();
	} else {
		yzDebug() << "Unable to open file " << mYzisinfo.name() << endl;
	}
}

void YZYzisinfo::writeYzisinfo() {
	int i;	
	if ( mYzisinfo.open( IO_WriteOnly ) ) {
		YZYzisinfoRecord * record;
		QTextStream write( &mYzisinfo );
		write.setEncoding( QTextStream::UnicodeUTF8 );

		for( i = YZSession::mYzisinfoList.count(); i > 0; --i ) {
			record = YZSession::mYzisinfoList[i - 1];

			if ( record->keyword() == "start_position" ) {
				write << "start_position ";
				write << dynamic_cast<YZYzisinfoRecordStartPosition*>(record)->position()->x();
				write << " "; 
				write << dynamic_cast<YZYzisinfoRecordStartPosition*>(record)->position()->y();
				write << " ";
				write << dynamic_cast<YZYzisinfoRecordStartPosition*>(record)->filename() << endl;
			}
		}
		
		for( i = YZSession::mYzisinfoList.count(); i > 0; --i ) {
			record = YZSession::mYzisinfoList[i - 1];
			
			if ( record->keyword() == "search_history" ) {
				int end = dynamic_cast<YZYzisinfoRecordSearchHistory*>(record)->indexCount();
				for( int j = 0; j < end; ++j ) {
					write << "search_history ";
					YZCursor * tmp = dynamic_cast<YZYzisinfoRecordSearchHistory*>(record)->previousSearchPosition();
					write << tmp->x();
					write << " ";
					write << tmp->y();
					write << " ";
					write << dynamic_cast<YZYzisinfoRecordSearchHistory*>(record)->filename() << endl;
				}
			}
		}
	      
		write << endl;
		mYzisinfo.close();
	}
}

YZCursor * YZYzisinfo::startPosition() {
	
	for ( YZYzisinfoList::Iterator it = YZSession::mYzisinfoList.begin();
	      it != YZSession::mYzisinfoList.end(); ++it ) {
		if ( (*it)->keyword() == "start_position" 
		&&   dynamic_cast<YZYzisinfoRecordStartPosition*>(*it)->filename() == YZSession::me->currentBuffer()->fileName() ) {
			return dynamic_cast<YZYzisinfoRecordStartPosition*>(*it)->position();
		}
	}

	return 0;
}

void YZYzisinfo::saveStartPosition( const QString & filename, const int x, const int y ) {

	for ( YZYzisinfoList::Iterator it = YZSession::mYzisinfoList.begin();
	      it != YZSession::mYzisinfoList.end(); ++it ) {
		if ( (*it)->keyword() == "start_position"
		&&   dynamic_cast<YZYzisinfoRecordStartPosition*>(*it)->filename() == filename ) {
			dynamic_cast<YZYzisinfoRecordStartPosition*>(*it)->setPosition(x, y);
			return;
		}
	}
	
	YZSession::mYzisinfoList.push_back( new YZYzisinfoRecordStartPosition( "start_position", filename, x, y ) ); 
	
	return;
}

YZCursor * YZYzisinfo::searchPosition() {
	
	for ( YZYzisinfoList::Iterator it = YZSession::mYzisinfoList.begin();
		   it != YZSession::mYzisinfoList.end(); ++it ) {
		if ( (*it)->keyword() == "search_history" ) {
			QString x = dynamic_cast<YZYzisinfoRecordSearchHistory*>(*it)->filename();
		}
		if ( (*it)->keyword() == "search_history" ) {
			if ( dynamic_cast<YZYzisinfoRecordSearchHistory*>(*it)->filename() == YZSession::me->currentBuffer()->fileName() ) {
				return dynamic_cast<YZYzisinfoRecordSearchHistory*>(*it)->previousSearchPosition();
			}
		}
	}
            
	return new YZCursor( YZSession::me->currentView()->getBufferCursor() );
}

void YZYzisinfo::saveSearchPosition( const QString & filename, const int x, const int y ) { 

	bool found = false;
	
	for ( YZYzisinfoList::Iterator it = YZSession::mYzisinfoList.begin(); it != YZSession::mYzisinfoList.end(); ++it ) {
		if ( (*it)->keyword() == "search_history" ) {
			if ( dynamic_cast<YZYzisinfoRecordSearchHistory*>(*it)->filename() == filename ) {
				found = true;
				dynamic_cast<YZYzisinfoRecordSearchHistory*>(*it)->setPosition( x, y );
				return;
			}
		}
	}
	         
	if ( ! found ) {         
		YZSession::mYzisinfoList.push_back( new YZYzisinfoRecordSearchHistory( "search_history", filename, x, y ) );
	}
	
	return;
}
