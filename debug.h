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

/** \file debug.h
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

//! \name Debug levels
//! @{
#define YZ_DEBUG_LEVEL 0        //!< debuggging level
#define YZ_WARNING_LEVEL 1      //!< warning level
#define YZ_ERROR_LEVEL 2        //!< error level
#define YZ_FATAL_LEVEL 3        //!< fatal level
//! @}

//! \name Debug areas
//! @{
#define UNSPECIFIED 	""          //!< unspecified debug area
#define CORE 			"libyzis"   //!< libyzis debug area
#define KYZIS 			"kyzis"     //!< kyzis debug area
#define NYZIS 			"nyzis"     //!< nyzis debug area
#define QYZIS 			"qyzis"     //!< qyzis debug area
#define AREA_TESTS 		"tests"     //!< tests debug area
//! @} 

/** default file name for controlling which area are enabled or not.
  * \see YZDebugBackend::parseRcfile()
  */
#define DEBUGRC_FNAME ".yzdebugrc" 

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
  * The call to parseArgv() and parseRcfile() need to be explicit, the
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
  * dbgHl() << "some debug data";
  * \endcode
  *
  */
class YZIS_EXPORT YZDebugBackend {
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

    /** Current global debugging level */ 
	int debugLevel() const { return _level; }

	/** All debug will be logged to the open file descriptor \p file.
      *
	  * stdout and stderr are perfectly valid file descriptors for this.
      */
	void setDebugOutput( FILE * file );

	/** Same as above, but just specifies a file name that 
      * will be opened instead of a file descriptor. 
      */
	void setDebugOutput( const QString& fileName);

	/** Set the debug \p level of an \p area 
      *
      * Once the debug level of an area has been set, it no longer
      * obeys the global debugging level.
      *
      * It is possible to use area and subarea. For example, "area1.subarea1"
      * is a sub-area of "area1". Subarea obeys the debugging level of the
      * area unless they are set individually.
      *
      * Example:
      * After:
      * setAreaLevel( "area1", YZ_WARNING_LEVEL );
      * setAreaLevel( "area1.subarea2", YZ_DEBUG_LEVEL );
      *
      * we have:
      * "area1": warning
      * "area1.area1": warning
      * "area1.area2": debug
      *
      * You can remove an area level with removeArea()
      */
	void setAreaLevel( const QString& area, int level );

	/** Returns the debugging level of an \p area. 
      *
      * If the area has not been assigned an individual debugging level,
      * returns the upmost area matching the beginning of area or if non
      * matches, the global level.
      */
	int areaLevel( const QString& area ) const;

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


    /** Parses argv to check for debug directives.
      *
      * \p argv must be converted into a string list. The argument that
      * are recognised are removed from the list so that further parsing by 
      * other modules can happen.
      *
      * Allowed directives are:
      * --level=debug|warning|error|fatal
      * --area-level=[area name],debug|warning|error|fatal
      * --debug-output=[filename]|stdout|stderr
      *
      * Example:
      * nyzis --level=warning --area-level=LuaEngine,debug --area-level=YZSession,debug
      */
    void parseArgv( QStringList & argv );

    /** Parses argc/argv to check for debug directives.
      *
      * The argument list is not modified. Internally, a QStringList
      * is build and the previous function is called
      */
    void parseArgv(int argc, char ** argv);

    /** Returns a string describing the YZDebugBackend current configuration
      */
    QString toString();

    /** A message handler for yzis to capture the error messages of Qt.
      *
      * The messages of Qt are printed on yzis log system, in the area
      * Qt, with the appropriate debugging level.
      */
    static void yzisMsgHandler( QtMsgType msgType, const char * text );

private:
    /** Initialise the YZDebugBackend(), set a few internal variables
      * (dictionaries, ...)
      */
    void init();

    /** Private constuctors for a singleton */
	YZDebugBackend();

    /** Singleton instance holder */
	static YZDebugBackend * _instance;

    /** debug level assigned to each area */
	QMap<QString,int> _areaLevel; 

    /** dictionnary from area string to area level */
	QMap<QString,int> _levelByName;

    /** dictionnary from area level to area string */
	QMap<int,QString> _nameByLevel;

    /** Global debugging level */
	int _level;

    /** File output of the debugging (may be stderr, stdout or any open file
      * descriptor */ 
	FILE * _output;
};


class YZDebugStream;

/** A function that manipulates the debug stream */
typedef YZDebugStream & (*YDBGFUNC)(YZDebugStream &);

/**
 * A Stream to send debug output.
 *
 * The output is stored internally in the YZDebugStream::output and flushed
 * using flush() when a '\\n' is sent or when a string ending in '\\n' is sent.
 */
class YZIS_EXPORT YZDebugStream {
	public:
		/** Constrctor for the stream.
          *
          * The debug \p area and debug \p level are fixed by stream. */
		YZDebugStream(const char * area="", int level=0);

        /** Destructor */
		~YZDebugStream();

		/** Operator to output a boolean */
		YZDebugStream& operator << (bool i);

		/** Operator to output a char */
		YZDebugStream& operator << (char i);

		/** Operator to output an unsigned char */
		YZDebugStream& operator << (unsigned char i);

		/** Operator to output a string */
		YZDebugStream& operator << (const QString& string);

		/** Operator to output a string list */
		YZDebugStream& operator << (const QStringList& string);

		/** Operator to output a string list */
		YZDebugStream& operator << (const char* string);

		/** Operator to output an int */
		YZDebugStream& operator << (int i);

		/** Operator to output an unsigned int */
		YZDebugStream& operator << (unsigned int i);

		/** Operator to output a long */
		YZDebugStream& operator << (long i);

		/** Operator to output an unsigned long */
		YZDebugStream& operator << (unsigned long i);

		/** Operator to output a short */
		YZDebugStream& operator << (short i);

		/** Operator to output a unsigned short */
		YZDebugStream& operator << (unsigned short i);

		/** Operator to output a unsigned double */
		YZDebugStream& operator << (double d);

		/** Operator that execute the YZDebugStream() manipulator function \p
          * f */
		YZDebugStream& operator << (YDBGFUNC f) {
			return ( *f )( *this );
		}

        /** Convenient sprintf function on debug streams */
        void sprintf( const char * fmt, ... );

        /** Flushes the current debug text to the debug output. 
          *
          * Most operator<<() will flush the output if it ends with '\\n'.
          * If not the output is stored internally in \ref output.
          */
		void flush();

	private:
		QString output;     //!< Temporary stored the debug output
		int level;          //!< debugging level of the stream
		QString area;       //!< debugging area of the stream

};

/** Output a \\n */
inline YZDebugStream & endl( YZDebugStream& s ) { s << "\n"; return s; }

/** Flush the debug stream */
inline YZDebugStream& flush( YZDebugStream& s ) { s.flush(); return s; }

/** Convenient function to build a debug stream.
  *
  * To use with: 
  * \code
  * yzDebug("some_area") << some_debug << some_more_debug << endl;
  * \endcode
  */
YZIS_EXPORT YZDebugStream yzDebug( const char * area = "" );

/** Convenient function to build a warning stream. */
YZIS_EXPORT YZDebugStream yzWarning( const char * area = "" );

/** Convenient function to build an error stream. */
YZIS_EXPORT YZDebugStream yzError( const char * area = "" );

/** Convenient function to build an fatal stream. */
YZIS_EXPORT YZDebugStream yzFatal( const char * area = "" );


//! \name Assertion macros
//! @{

/** Truth assertion with custom message.
  *
  * The assertion will automatically include file:line information and
  * copy of the assertion.
  *
  * If the assertion fails, an error is displayed in the empty area.
  */
#define YZASSERT_MSG( assertion, msg ) { if (! (assertion) ) { yzError() << QString("%1:%2 assertion '%3' failed : %4\n").arg(__FILE__).arg( __LINE__).arg(#assertion).arg( msg ); } }

/** Truth assertion with empty message.
  *
  * The assertion will automatically include file:line information and
  * copy of the assertion.
  *
  * If the assertion fails, an error is displayed in the empty area.
  */
#define YZASSERT( assertion ) YZASSERT_MSG( assertion, "" )


/** Equality assertion
  *
  * The assertion will automatically include file:line information and
  * copy of the assertion.
  *
  * If the assertion fails, an error is displayed in the empty area.
  */
#define YZASSERT_EQUALS( a, b ) { if (a != b) { yzError() << QString("%1:%2 - %3 == %4 failed : '%5' != '%6'\n").arg(__FILE__).arg( __LINE__).arg(#a).arg(#b).arg(a).arg(b); } }

//! @}

//! \name Convenient macros for debugging
//! @{
/** qPrintable shortcut, convenient when debugging. */
#define qp(s)   qPrintable(s)

/** Function + line, very convenient when debugging */
#define HERE() qp(QString("%1:%2 ").arg(__PRETTY_FUNCTION__).arg(__LINE__))

/** File + line location, convenient when debugging.
  * It makes anyting printable, from a QString to a QDate. */
#define LOCATION() qp(QString("%1:%2 ").arg(__FILE__).arg(__LINE__))

//! @}

#endif /* YZ_DEBUG_H */
