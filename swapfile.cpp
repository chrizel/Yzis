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
#include <qfile.h>
#include <qtextstream.h>

YZSwapFile::YZSwapFile(YZBuffer *b) {
	mParent = b;
	mFilename = b->fileName()+".ywp";
	//check if the file exists and then popup an error XXX
	if ( QFile::exists( mFilename ) ) yzDebug() << "Swap file already EXISTS ! " << endl;
	//then start a recover session ? XXX
}

void YZSwapFile::setFileName( const QString& fname ) {
	unlink();
	mFilename = fname;
}

void YZSwapFile::flush() {
	yzDebug() << "Flushing swap to " << mFilename << endl;
	QFile f( mFilename );
	if ( f.open( IO_WriteOnly | IO_Raw | IO_Append ) ) { //open at end of file
		QTextStream stream( &f );
		if ( !mHistory.empty() ) {
			for ( QValueList<swapEntry>::iterator it = mHistory.begin(); it != mHistory.end(); ++it ) {
				stream << ( *it ).modifiers << "," << ( *it ).inputs << ",";
			}
		}
		f.close();
	} else {
		//XXX error
	}
	mHistory.clear(); //clear previous history
	//dump registers XXX
	//add Yzis Version
	//datetime creation of the stamp file
	//PID of the process
	//name of the original file name (since a swap file can be named differently)
}

void YZSwapFile::addToSwap( int inputs, int modifiers ) {
	swapEntry e = { inputs, modifiers };
	mHistory.append( e );
	if ( mHistory.size() >= 20 ) flush();
}

void YZSwapFile::unlink() {
	yzDebug() << "Unlink swap file " << mFilename << endl;
	if ( QFile::exists( mFilename ) )
		QFile::remove ( mFilename );
}

