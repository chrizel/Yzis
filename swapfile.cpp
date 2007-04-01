/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTime>
#include <sys/types.h>
#include <sys/stat.h>

#include "debug.h"
#include "swapfile.h"
#include "yzis.h"
#include "internal_options.h"
#include "buffer.h"
#include "action.h"
#include "view.h"
#include "session.h"
#include "portability.h"

#define dbg()    yzDebug("YZSwapFile")
#define err()    yzError("YZSwapFile")

YZSwapFile::YZSwapFile(YZBuffer *b) {
	mParent = b;
	mRecovering = false;
	mFilename = QString();
	setFileName( b->fileName() );
	mNotResetted=true;
	//init();
}

void YZSwapFile::setFileName( const QString& fname ) {
	dbg() << "setFileName( " << fname << ")" << endl;
	unlink();
	mFilename = fname.section( '/', 0, -2 ) + "/." + fname.section( '/', -1 ) + ".ywp";
	dbg() << "Swap filename = " << mFilename << endl;
}

void YZSwapFile::flush() {
	if ( mRecovering ) return;
	if ( mParent->getLocalIntegerOption("updatecount") == 0 ) return;
	if ( mNotResetted ) init();
	dbg() << "Flushing swap to " << mFilename << endl;
	QFile f( mFilename );
	struct stat buf;
#ifndef YZIS_WIN32_GCC
	int i = lstat( mFilename.toLocal8Bit(), &buf );
	if ( i != -1 && S_ISREG( buf.st_mode ) && !S_ISLNK( buf.st_mode ) && buf.st_uid == geteuid() 
#else
	if ( true
#endif
		&& f.open( QIODevice::WriteOnly | QIODevice::Append ) ) { //open at end of file
#ifndef YZIS_WIN32_GCC
		chmod( mFilename.toLocal8Bit(), S_IRUSR | S_IWUSR );
#endif
		QTextStream stream( &f );
		if ( !mHistory.empty() ) {
			for ( int ab = 0 ; ab < mHistory.size(); ++ab )
				stream << mHistory.at(ab).type << mHistory.at(ab).col <<","<< mHistory.at(ab).line <<","<< mHistory.at(ab).str << endl;
		}
		f.close();
	} else {
		YZSession::self()->popupMessage(_( "Warning, the swapfile could not be opened maybe due to restrictive permissions." ));
		mNotResetted = true;//don't try again ...
	}
	mHistory.clear(); //clear previous history
}

void YZSwapFile::addToSwap( YZBufferOperation::OperationType type, const QString& str, unsigned int col, unsigned int line ) {
	if ( mRecovering ) return;
	if ( mParent->getLocalIntegerOption("updatecount") == 0 ) return;
	swapEntry e;
	e.type = type;
	e.col = col;
	e.line = line;
	e.str = str;
	mHistory.append( e );
	if ( ( ( int )mHistory.size() ) >= mParent->getLocalIntegerOption("updatecount") ) flush();
}

void YZSwapFile::unlink() {
	dbg() << "Unlink swap file " << mFilename << endl;
	if ( ! mFilename.isNull() && QFile::exists( mFilename ) )
		QFile::remove ( mFilename );
	mNotResetted=true;
}

void YZSwapFile::init() {
	dbg() << "init() mFilename=" << mFilename << endl;
	if ( QFile::exists( mFilename ) ) {
		dbg() << "Swap file already EXISTS ! " << endl;
		//that should really not happen ...
		mNotResetted = true; //don't try to access that file later ...
		return;
	}

	QFile f( mFilename );
	if ( f.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
		chmod( mFilename.toLocal8Bit(), S_IRUSR | S_IWUSR );
		QTextStream stream( &f );
		stream << "WARNING : do not edit, this file is a temporary file created by Yzis and used to recover files in case of crashes" << endl << endl;
		stream << "Generated by Yzis " << VERSION_CHAR << endl;
		stream << "Edited file: " << mParent->fileName() << endl;
		stream << "Creation date: " << QDateTime::currentDateTime().toString() << endl;
// XXX	stream << "Process ID: " << QString::number( getpid() ) << endl;
		stream << endl << endl << endl;
		f.close();
	} else {
		YZSession::self()->popupMessage(_( "Warning, the swapfile could not be created maybe due to restrictive permissions." ));
		mNotResetted = true;
		return;
	}
	mNotResetted = false;
}

bool YZSwapFile::recover() {
	mRecovering=true;
	QFile f( mFilename );
	if ( f.open( QIODevice::ReadOnly ) ) {
		QTextStream stream( &f );
		while ( !stream.atEnd() ) {
			QString line = stream.readLine();
				//stream << ( *it ).type << ( *it ).col <<","<< ( *it ).line <<","<< ( *it ).str << endl;
			QRegExp rx("([0-9])([0-9]*),([0-9]*),(.*)");
			if ( rx.exactMatch( line ) ) {
				replay( ( YZBufferOperation::OperationType )rx.cap( 1 ).toInt(), rx.cap( 2 ).toUInt(), rx.cap( 3 ).toUInt(), rx.cap( 4 ) );
			} else {
				dbg() << "Error replaying line: " << line << endl;
			}
		}
		f.close();
	} else {
		YZSession::self()->popupMessage(_( "The swap file could not be opened, there will be no recovering for this file, you might want to check permissions of files." ));
		mRecovering=false;
		return false;
	}

	mRecovering=false;
	return true;
}

void YZSwapFile::replay( YZBufferOperation::OperationType type, unsigned int col, unsigned int line, const QString& text ) {
	YZView *pView = mParent->firstView();
	pView->setPaintAutoCommit(false);
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
			mParent->action()->deleteLine( pView, line, 1, QList<QChar>() );
			break;
	}
	pView->commitPaintEvent();
}

