/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>
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

#include "debug.h"
#include "swapfile.h"
#include "yzis.h"
#include "internal_options.h"
#include "buffer.h"
#include "action.h"
#include "view.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qdatetime.h>
#include <sys/types.h>
#include <unistd.h>

YZSwapFile::YZSwapFile(YZBuffer *b) {
	mParent = b;
	mRecovering = false;
	mFilename = b->fileName()+".ywp";
	mNotResetted=true;
	init();
}

void YZSwapFile::setFileName( const QString& fname ) {
	yzDebug() << "Swap change filename " << fname << endl;
	unlink();
	mFilename = fname;
}

void YZSwapFile::flush() {
	if ( mRecovering || mNotResetted ) return;
	yzDebug() << "Flushing swap to " << mFilename << endl;
	QFile f( mFilename );
	if ( f.open( IO_WriteOnly | IO_Raw | IO_Append ) ) { //open at end of file
		QTextStream stream( &f );
		if ( !mHistory.empty() ) {
			for ( QValueList<swapEntry>::iterator it = mHistory.begin(); it != mHistory.end(); ++it ) {
				stream << ( *it ).type << ( *it ).col <<","<< ( *it ).line <<","<< ( *it ).str << endl;
			}
		}
		f.close();
	} else {
		YZSession::me->popupMessage(tr( "Warning, the swapfile could not be opened maybe due to restrictive permissions." ));
		mNotResetted = true;//dont try again ...
	}
	mHistory.clear(); //clear previous history
}

void YZSwapFile::addToSwap( YZBufferOperation::OperationType type, const QString& str, unsigned int col, unsigned int line ) {
	if ( mRecovering || mNotResetted ) return;
	swapEntry e = { type, col, line, str };
	mHistory.append( e );
	if ( ( ( int )mHistory.size() ) >= mParent->getLocalIntOption("updatecount") ) flush();
}

void YZSwapFile::unlink() {
	yzDebug() << "Unlink swap file " << mFilename << endl;
	if ( QFile::exists( mFilename ) )
		QFile::remove ( mFilename );
}

void YZSwapFile::init() {
	yzDebug() << "Swap : init file " << mFilename << endl;
	if ( QFile::exists( mFilename ) ) {
		yzDebug() << "Swap file already EXISTS ! " << endl;
		//that should really not happen ...
		mNotResetted = true; //don't try to access that file later ...
		return;
	}

	QFile f( mFilename );
	if ( f.open( IO_WriteOnly | IO_Raw | IO_Truncate ) ) {
		QTextStream stream( &f );
		stream << "WARNING : do not edit, this file is a temporary file created by Yzis and used to recover files in case of crashes" << endl << endl;
		stream << "Generated by Yzis " << VERSION_CHAR << endl;
		stream << "Edited file : " << mParent->fileName() << endl;
		stream << "Creation date : " << QDateTime::currentDateTime().toString() << endl;
		stream << "Process ID : " << QString::number( getpid() ) << endl;
		stream << endl << endl << endl;
		f.close();
	} else {
		YZSession::me->popupMessage(tr( "Warning, the swapfile could not be created maybe due to restrictive permissions." ));
		mNotResetted = true;
		return;
	}
	mNotResetted = false;
}

bool YZSwapFile::recover() {
	mRecovering=true;
	QFile f( mFilename );
	if ( f.open( IO_ReadOnly ) ) {
		QTextStream stream( &f );
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
				//stream << ( *it ).type << ( *it ).col <<","<< ( *it ).line <<","<< ( *it ).str << endl;
			QRegExp rx("([0-9])([0-9]*),([0-9]*),(.*)");
			if ( rx.exactMatch( line ) ) {
				replay( ( YZBufferOperation::OperationType )rx.cap( 1 ).toInt(), rx.cap( 2 ).toUInt(), rx.cap( 3 ).toUInt(), rx.cap( 4 ) );
			} else {
				yzDebug() << "Error replaying line : " << line << endl;
			}
		}
		f.close();
	} else {
		YZSession::me->popupMessage(tr( "The swap file could not be opened, there will be no recovering for this file, you might want to check permissions of files." ));
		mRecovering=false;
		return false;
	}

	mRecovering=false;
	return true;
}

void YZSwapFile::replay( YZBufferOperation::OperationType type, unsigned int col, unsigned int line, const QString& text ) {
	YZView *pView = mParent->firstView();
	switch( type ) {
		case YZBufferOperation::ADDTEXT:
			mParent->action()->insertChar( pView, col, line, text ); 
			break;
		case YZBufferOperation::DELTEXT: 
			mParent->action()->deleteChar( pView, col, line, text.length() );
			break;
		case YZBufferOperation::ADDLINE:
			mParent->action()->insertNewLine( pView, 0, line );
			break;
		case YZBufferOperation::DELLINE:
			mParent->action()->deleteLine( pView, line, 1 );
			break;
	}
}

