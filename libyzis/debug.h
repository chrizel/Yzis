/* This file is part of the Yzis libraries
 *  Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>
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
 * This file was mostly inspired from the kdelibs kdDebug class distributed under LGPL by 
 * by the KDE project.
 * Here are the corresponding copyrights owner :
 * 1997 Matthias Kalle Dalheimer ( kalle@kde.org )
 * 2000-2002 Stephan Kulow ( coolo@kde.org )
 * 2002 Holger Freyther ( freyther@kde.org )
 */

#ifndef YZ_DEBUG_H
#define YZ_DEBUG_H
/**
 * $Id$
 */

#include <qstring.h>
#include <qfile.h>

class QStringList;
class QCString;

/**
 * Level of debug
 */
#define YZ_DEBUG_LEVEL 0
#define YZ_WARNING_LEVEL 1
#define YZ_ERROR_LEVEL 2
#define YZ_FATAL_LEVEL 3

/**
 * Areas of debug
 */
#define UNSPECIFIED 0
#define CORE 1
#define KYZIS 2
#define NYZIS 3

class YZDebugStream;

typedef YZDebugStream & (*YDBGFUNC)(YZDebugStream &); // manipulator function

/**
 * THE debug class ;)
 */

class YZDebugStream {
	public:
		// where to output the debug
		YZDebugStream(int area=0, int level=0);
/*		YZDebugStream(YZDebugStream& str ) :
			output( str.output ), area( str.area ),level( str.level ) { str.output.truncate( 0 ); }
		YZDebugStream(const& YZDebugStream& str ) :
			output( str.output ), area( str.area ),level( str.level ) { }*/
		~YZDebugStream();

		//operators
		YZDebugStream& operator << (bool i);
		YZDebugStream& operator << (char i);
		YZDebugStream& operator << (unsigned char i);
		YZDebugStream& operator << (const QCString& string);
		YZDebugStream& operator << (const QString& string);
		YZDebugStream& operator << (const QStringList& string);
		YZDebugStream& operator << (const char* string);
		YZDebugStream& operator << (int i);
		YZDebugStream& operator << (unsigned int i);
		YZDebugStream& operator << (long i);
		YZDebugStream& operator << (unsigned long i);
		YZDebugStream& operator << (short i);
		YZDebugStream& operator << (unsigned short i);
		YZDebugStream& operator << (double d);
		YZDebugStream& operator << (YDBGFUNC f) {
			return ( *f )( *this );
		}


		void flush();

	private:
		QString output;
		int area,level;
	
};

inline YZDebugStream &endl( YZDebugStream& s ) { s << "\n"; return s; }
inline YZDebugStream& flush( YZDebugStream& s ) { s.flush(); return s; }

//global functions (if it reminds you KDE it's not pure hasard :)
YZDebugStream yzDebug( int area = 0 );
YZDebugStream yzWarning( int area = 0 );
YZDebugStream yzError( int area = 0 );
YZDebugStream yzFatal( int area = 0 );

// Assertion
#define YZASSERT_MSG( assertion, msg ) { if (! (assertion) ) { yzError() << QString("%1:%2 assertion '%3' failed : %4\n").arg(__FILE__).arg( __LINE__).arg(#assertion).arg( msg ); } }
#define YZASSERT( assertion ) YZASSERT_MSG( assertion, "" )

#endif /* YZ_DEBUG_H */
