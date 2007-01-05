/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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

/**
 * This file was mostly inspired from the kdelibs kdDebug class distributed under LGPL
 * by the KDE project.
 * Here are the corresponding copyrights owner :
 * 1997 Matthias Kalle Dalheimer ( kalle@kde.org )
 * 2000-2002 Stephan Kulow ( coolo@kde.org )
 * 2002 Holger Freyther ( freyther@kde.org )
 */

#ifndef YZ_DEBUG_H
#define YZ_DEBUG_H

#include <QString>
#include <QMap>
#include "yzismacros.h"

class QStringList;

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
#define UNSPECIFIED 	""
#define CORE 			"libyzis"
#define KYZIS 			"kyzis"
#define NYZIS 			"nyzis"
#define QYZIS 			"qyzis"
#define AREA_TESTS 		"tests"

#define FILENAME_DEBUGRC ".yzdebugrc"

/** Log system used for debugging.
  *
  * Our debugging system supports 4 levels:
  * - YZ_DEBUG_LEVEL: shows debugging output, warning and errors
  * - YZ_WARNING_LEVEL: shows warnings and errors, debug output is disabled
  * - YZ_ERROR_LEVEL: shows only errors, debug and warnings are disabled.
  * - YZ_FATAL_LEVEL: shows only fatal errors
  * 
  *
  * The debug output may be:
  * - sent to a specified filename
  * - sent to a file in the temp directory
  * - sent to the stderr
  * - sent to the stdout
  * This is controlled by setDebugOutput()
  *
  * Moreover, in order to ease the debugging process, it is possible to define
  * areas, and to assign an individual debugging level to each of them with
  * setAreaLevel().
  *
  * This allows for example to have the rendering code only display errors but
  * keep all the debugging output of the scripting engine. Or the opposite.
  *
  * Because no two developers will need the same configuration, the debug
  * level and areas can be configured dynamically through two methods:
  * - command line arguments: see parseArgv()
  * - .yzdebugrc file in the current directory: see parseRcfile()
  *
  * The calls to parseArgv() and parseRcfile() need to be explicit, the
  * framework does not do any automatic action.
  *
  * By default, the debug framework will start with no areas, debug level set
  * to debug in debug mode (DEBUG symbol defined) or warning in release mode
  * (DEBUG symbol not defined). 
  *
  * The default output is a file in the /tmp directory named yzisdebug.log on
  * windows and yzisdebug-[your user name].log on unix.
  *
  * The framework gives a very flexible approach for debugging.
  *
  * When you are writing code, you should first define an area for your code
  * and then use the stream yzDebug(area) to output your debug information.
  *
  * My recommendation is to have a define at the beginning of your source file
  * to abbreviate the debug writing command:
  * \code
  * #define dbgHl() (yzDebug("SyntaxHighlighting"))
  *
  * dbgHl() << "some debug intput";
  * \endcode
  *
  */
class YZDebugBackend {
public:
	~YZDebugBackend();

    /** The singleton constructor */
	static YZDebugBackend * instance();

	/** Write data to the debug backend. 
      *
      * Note that Qt provides a macro qPrintable that makes mostly
      * anything printable as a const char * . This can be convenient.
      *
      * \param level the debugging level
      * \param area  the area
      * \param data  the data to display
      */
	void flush( int level, const QString& area, const char * data );

	/** Set debugging level.
      *
      * \param level can YZ_DEBUG_LEVEL, YZ_ERROR_LEVEL or YZ_FATAL_LEVEL
	  */
	void setDebugLevel( int level ) { _level = level; }

    /** Current debugging level */ 
	int debugLevel() const { return _level; }

	/** All debug will be logged to the open file descriptor \p file.
      *
	  * stdout and stderr are perfectly valid file descriptors for this.
      */
	void setDebugOutput( FILE * file );

	/** Same as above, but just specifies a file name that 
      * will be opened instead of a file descriptor. 
      */
	void setDebugOutput( const QString& );

	/** Set the debug \p level of an \p area 
      *
      * Once the debug level of an area has been set, it no longer
      * obeys the global debugging level.
      *
      * You can remove an area level with removeArea()
      */
	void setAreaLevel( const QString& area, int level ) {
		_areaLevel[area] = level;
	}

	/** Returns the debugging level of an \p area. 
      *
      * If the area has not been assigned an individual debugging level,
      * returns the global level.
      */
	int areaLevel( const QString& area ) const {
        return _areaLevel.value(area, debugLevel());
	}

    /** Remove the individual debugging level of this area.
      *
      * The area now obeys the global debugging level.
      */
    void removeArea(const QString & area) {
        _areaLevel.remove( area );
    }

	/** Remove all debug areas. 
      *
      * Debug value returns to the global value.
      */
	void clearArea() { _areaLevel.clear(); }

	/** Read a rcfile to adjust area output and debug level
      *
	  * The syntax is:
      * "level" ":" "debug" | "warning" | "error"
      *  [area]   ":" "debug" | "warning" | "error"
      * "output" ":" [filename]
	  **/
	void parseRcfile(const char * filename);

    /** Parse default rc file.
      *
      * The default rc file is a file named .yzdebugrc located in the
      * directory where yzis is executed.
      */
	void parseRcfile();


    /** Parses argc/argv to check for debug directives.
      *
      * Allowed directives are:
      * --level=debug|warning|error|fatal
      * --area-level=[area name],debug|warning|error|fatal
      *
      * Example:
      * nyzis --level=warning --area-level=LuaEngine,debug --area-level=YZSession,debug
      */
    void parseArgv(int & argc, char ** argv);

    /** Returns a string describing the YZDebugBackend current configuration
      */
    QString toString();

private:
    void init();

	YZDebugBackend();
	static YZDebugBackend * _instance;

	QMap<QString,int> _areaLevel;
	QMap<QString,int> _levelByName;
	QMap<int,QString> _nameByLevel;
	int _level;
	FILE * _output;
};


class YZDebugStream;

typedef YZDebugStream & (*YDBGFUNC)(YZDebugStream &); // manipulator function

/**
 * THE debug class ;)
 */

class YZIS_EXPORT YZDebugStream {
	public:
		// where to output the debug
		YZDebugStream(const char * area="", int level=0);
/*		YZDebugStream(YZDebugStream& str ) :
			output( str.output ), area( str.area ),level( str.level ) { str.output.truncate( 0 ); }
		YZDebugStream(const& YZDebugStream& str ) :
			output( str.output ), area( str.area ),level( str.level ) { }*/
		~YZDebugStream();

		//operators
		YZDebugStream& operator << (bool i);
		YZDebugStream& operator << (char i);
		YZDebugStream& operator << (unsigned char i);
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

        /** Convenient sprintf function on debug streams */
        void sprintf( const char * fmt, ... );

		void flush();

	private:
		QString output;
		int level;
		QString area;

};

inline YZDebugStream &endl( YZDebugStream& s ) { s << "\n"; return s; }
inline YZDebugStream& flush( YZDebugStream& s ) { s.flush(); return s; }

//global functions (if it reminds you KDE it's not pure hasard :)
YZIS_EXPORT YZDebugStream yzDebug( const char * area = "" );
YZIS_EXPORT YZDebugStream yzWarning( const char * area = "" );
YZIS_EXPORT YZDebugStream yzError( const char * area = "" );
YZIS_EXPORT YZDebugStream yzFatal( const char * area = "" );

// Assertion
#define YZASSERT_MSG( assertion, msg ) { if (! (assertion) ) { yzError() << QString("%1:%2 assertion '%3' failed : %4\n").arg(__FILE__).arg( __LINE__).arg(#assertion).arg( msg ); } }
#define YZASSERT( assertion ) YZASSERT_MSG( assertion, "" )
#define YZASSERT_EQUALS( a, b ) { if (a != b) { yzError() << QString("%1:%2 - %3 == %4 failed : '%5' != '%6'\n").arg(__FILE__).arg( __LINE__).arg(#a).arg(#b).arg(a).arg(b); } }

/** qPrintable shortcut, convenient when debugging. */
#define qp(s)   qPrintable(s)

/** Function + line, very convenient when debugging */
#define HERE() qp(QString("%1:%2 ").arg(__PRETTY_FUNCTION__).arg(__LINE__))

/** File + line location, convenient when debugging */
#define LOCATION() qp(QString("%1:%2 ").arg(__FILE__).arg(__LINE__))


#endif /* YZ_DEBUG_H */
