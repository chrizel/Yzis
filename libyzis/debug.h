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
 *
 * Some improvements have been made to support area, and more different kind
 * of debug output.
 *
 * See also http://www.yzis.org/Debugging.
 */

#ifndef YZ_DEBUG_H
#define YZ_DEBUG_H

#include <QString>
#include <QMap>
#include <QFile>
#include "yzismacros.h"
#include "noncopyable.h"

#include <cstdio>

class QStringList;

//! \name Debug levels
//! @{
#define YZ_DEEPDEBUG_LEVEL   0  //!< deep debuggging level
#define YZ_DEBUG_LEVEL   1      //!< debuggging level
#define YZ_WARNING_LEVEL 2      //!< warning level
#define YZ_ERROR_LEVEL   3      //!< error level
#define YZ_FATAL_LEVEL   4      //!< fatal level 
//! @}

/** default file name for controlling which area are enabled or not.
  * \see YDebugBackend::parseRcfile()
  */
#define DEBUGRC_FNAME ".yzdebugrc"

/** Log system used for debugging.
  *
  * YDebugBackend is a singleton instance that manages the debugging log,
  * levels and area.
  *
  * Our debugging system supports 4 levels:
  * - YZ_FATAL_LEVEL: shows only fatal errors
  * - YZ_ERROR_LEVEL: shows only errors. Debug and warnings are disabled.
  * - YZ_WARNING_LEVEL: shows warnings and errors. Debug output is disabled
  * - YZ_DEBUG_LEVEL: shows regular debugging output, warning and errors
  * - YZ_DEEPDEBUG_LEVEL: shows deep debugging output, warning and errors.
  *
  * Levels can be assigned globally and to specific log areas.
  *
  * The debug output may be sent to:
  * - a specified filename
  * - a file in the temp directory
  * - the stderr
  * - the stdout
  *
  * The debug information that you display in your code should be assigned to
  * an area. An area is just a hierarchy of names, separated by dots. With
  * area (and subarea), it is possible to have one part of Yzis in debug mode
  * and the other part in error mode.
  *
  * The debug level and areas can be configured dynamically through two methods:
  * - command line arguments of Yzis: see parseArgv()
  * - .yzdebugrc file in the current directory: see parseRcfile()
  *
  * The call to parseArgv() and parseRcfile() must be done early in the
  * initialisation process of Yzis.
  *
  * By default, the debug framework will start with no areas, debug level set
  * to debug when compiled to debug (DEBUG symbol defined) or warning when
  * compiled for release (DEBUG symbol not defined). 
  *
  * The default output is a file in the /tmp directory named yzisdebug.log on
  * windows and yzisdebug-[your_user_name].log on unix. On windows, the debug
  * log is also sent to the standard debugging stream (with
  * OutputDebugString() ).
  *
  * The simple way to spy the debug log on unix is to do:
  * tail -f /tmp/yzisdebug-[your_user_name].log
  *
  * <b>Important note for windows users</b><br>
  * On windows, there is a bug where there is two instances of the singleton
  * YDebugBackend(). The consequence is that the debug log is not properly
  * redirected and spying the /tmp/yzdebug.log will also create problems
  * because windows can not create a new file when the file when the same name
  * is still being read.
  *
  * The proper way to catch debug output on windows is to use DebugView from
  * <a href="http://www.sysinternals.com">sys internals</a>.
  *
  * In practice, the log interface is used through the functions yzDebug(),
  * yzWarning() and yzError(). Everything else is done in the background.
  *
  * When you are writing code, you should first define an area for your code
  * and then use the stream yzDebug(area) to output your debug information.
  *
  * My recommendation is to have a define at the beginning of your source file
  * to abbreviate the debug writing command:
  *
  * \code
  * #define dbg() (yzDebug("SyntaxHighlighting"))
  * #define err() (yzError("SyntaxHighlighting"))
  *
  * void some_function(int a)
  * {
  *     dbg() << "some_function was called." << endl;
  *     dbg().SPrintf("a=%d", a );
  *     // do some stuff
  *     if (something_goes_wrong) {
  *        err() << "There is a problem!" << endl;
  *        return;
  *     }
  *     // do more stuff
  *     return;
  * }
  * \endcode
  *
  */
class YZIS_EXPORT YDebugBackend : private boost::noncopyable
{

public:

    /** The singleton constructor */
    static YDebugBackend * self();

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
    void setDebugLevel( int level )
    {
        _level = level;
    }

    /** Current global debugging level */
    int debugLevel() const
    {
        return _level;
    }

    /** All debug will be logged to the file \p fileName
	 *  \p fileName may be stdout or stderr.
     */
    void setDebugOutputFilename( const QString& fileName);

	/** Temporary debug output
	 */
    void setDebugOutputTemp();


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
         * \code
         * setAreaLevel( "area1", YZ_WARNING_LEVEL );
         * setAreaLevel( "area1.subarea2", YZ_DEBUG_LEVEL );
         * \endcode
         *
         * we have:
         * \code
         * "area1": warning
         * "area1.subarea1": warning
         * "area1.subarea2": debug
         * \endcode
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
    void removeArea(const QString & area)
    {
        _areaLevel.remove( area );
    }

    /** Remove all debug areas.
         *
         * Debug value returns to the global value.
         */
    void clearArea()
    {
        _areaLevel.clear();
    }

    /** Read a rcfile to adjust area output and debug level
       *
      * The syntax is:
       * \li "level" ":" "debug" | "warning" | "error" | "deepdebug" <br>
       * \li [area] ":" "debug" | "warning" | "error" | "deepdebug" <br>
       * \li "output" ":" [filename]<br>
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
      * \li --level=debug|warning|error|fatal
      * \li --area-level=[area name],debug|warning|error|fatal
      * \li --debug-output=[filename]|stdout|stderr
      *
      * Example:
      * nyzis --level=warning --area-level=LuaEngine,debug --area-level=YSession,debug
      */
    void parseArgv( QStringList & argv );

    /** Parses argc/argv to check for debug directives.
      *
      * The argument list is not modified. Internally, a QStringList
      * is build and the previous function is called
      */
    void parseArgv(int argc, char ** argv);

    /** Returns a string describing the YDebugBackend current configuration
      */
    QString toString();

    /** A message handler for yzis to capture the error messages of Qt.
      *
      * The messages of Qt are printed on yzis log system, in the area
      * Qt, with the appropriate debugging level.
      */
    static void yzisMsgHandler( QtMsgType msgType, const char * text );

    /** Allocate memory for the object from libyzis.
      *
      * See \ref yzmalloc for details.
      */
    void * operator new( size_t tSize );

    /** Free memory of the object from libyzis.
      *
      * See \ref yzmalloc for details.
      */
    void operator delete( void* p );

private:
    /** Initialise the YDebugBackend(), set a few internal variables
      * (dictionaries, ...)
      */
    void init();

	void closeOutput();

    /** Private constructors for a singleton */
    YDebugBackend();

    /** Private copy constructors for a singleton */
    YDebugBackend( YDebugBackend & other);

    /** Private copy assignment for a singleton */
    YDebugBackend & operator=( YDebugBackend & other );

    /** Private destructor for a singleton */
    ~YDebugBackend();

    /** Singleton instance holder */
    static YDebugBackend * me;

    /** debug level assigned to each area */
    QMap<QString, int> _areaLevel;

    /** dictionnary from area string to area level */
    QMap<QString, int> _levelByName;

    /** dictionnary from area level to area string */
    QMap<int, QString> _nameByLevel;

    /** Global debugging level */
    int _level;

    /** File output of the debugging (may be stderr, stdout or any open file
      * descriptor */
    QFile* _output;
};

/** Special function to allocate memory.
  *
  * The need for this function is due to the memory handling of shared
  * libraries in windows. On windows, objects allocated in shared libraries
  * must be freed from the shared library. To make it happen, Yzis provides
  * its own yzmalloc() and yzfree() functions. The only thing they do is call
  * the C library real malloc/free. But from the right place.
  *
  * In order to make <i>deletion from shared library</i> happen for objects
  * too, some objects in libyzis have a new() and delete() operator declared.
  * Those new/delete will call yzmalloc() and yzfree().
  */
void * yzmalloc( size_t tSize );

/** Free memory allocated by yzmalloc(). */
void yzfree( void * p );

class YDebugStream;

/** A function that manipulates the debug stream */
typedef YDebugStream & (*YDBGFUNC)(YDebugStream &);

/**
 * A Stream to send debug output.
 *
 * The output is stored internally in the YDebugStream::output and flushed
 * using flush() when a '\\n' is sent or when a string ending in '\\n' is sent.
 *
 * This is the standard way of writing log statements in Yzis. The
 * YDebugStream supports printf like syntax with SPrintf(), or more classical
 * C++ stream. 
 *
 * The constructor specifies the area and level of logging. 
 *
 * The
 * output is flushed to the log system each time a \\n is encountered, or
 * automatically after a SPrintf().
 *
 * The functions yzDebug() and yzError() return an YDebugStream() with the
 * right debugging level and area set.
 *
 * Typical code looks like this.
 * \code
 * #define dbg() yzDebug("area1", YZ_DEBUG_LEVEL)
 * #define err() yzError("area1", YZ_DEBUG_LEVEL)
 *
 * int some_func( int * p, int q )
 * {
 *    if (p == NULL) {
 *      err() << "p is NULL" << endl;
 *      return 0;
 *    }
 *
 *    dbg().SPrintf("*p=%d, q=%d", *p, q );
 *
 *    // do some stuff
 *
 *    return q;
 * }
 * \endcode
 *
 * The YDebugStream supports the display of many types of variables. Qt also
 * provides a qPrintable() macro, that converts any Qt type into a const char
 * *. In Yzis, we provide the qp() macro that calls qPrintable() but is
 * shorter to type.
 *
 * There are also a few convenience macro:
 * - HERE() : displays the name of the current function and position in the
 * current file
 *
 */
class YZIS_EXPORT YDebugStream
{
public:
    /** Constrctor for the stream.
            *
            * The debug \p area and debug \p level are fixed by stream. */
    explicit YDebugStream(const char * area = "", int level = 0);

    /** Destructor */
    ~YDebugStream();

    /** Operator to output a boolean */
    YDebugStream& operator << (bool i);

    /** Operator to output a char */
    YDebugStream& operator << (char i);

    /** Operator to output an unsigned char */
    YDebugStream& operator << (unsigned char i);

    /** Operator to output a string */
    YDebugStream& operator << (const QString& string);

    /** Operator to output a string list */
    YDebugStream& operator << (const QStringList& string);

    /** Operator to output a string list */
    YDebugStream& operator << (const char* string);

    /** Operator to output an int */
    YDebugStream& operator << (int i);

    /** Operator to output an unsigned int */
    YDebugStream& operator << (unsigned int i);

    /** Operator to output a long */
    YDebugStream& operator << (long i);

    /** Operator to output an unsigned long */
    YDebugStream& operator << (unsigned long i);

    /** Operator to output a short */
    YDebugStream& operator << (short i);

    /** Operator to output a unsigned short */
    YDebugStream& operator << (unsigned short i);

    /** Operator to output a unsigned double */
    YDebugStream& operator << (double d);

    /** Operator that execute the YDebugStream() manipulator function \p
            * f */
    YDebugStream& operator << (YDBGFUNC f)
    {
        return ( *f )( *this );
    }

    /** Convenient SPrintf function on debug streams */
    void SPrintf( const char * fmt, ... );

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
inline YDebugStream & endl( YDebugStream& s )
{
    s << "\n"; return s;
}

/** Flush the debug stream */
inline YDebugStream& flush( YDebugStream& s )
{
    s.flush(); return s;
}

/** Convenient function to build a debug stream.
  *
  * To use with: 
  * \code
  * yzDebug("some_area") << some_debug << some_more_debug << endl;
  * \endcode
  */
YZIS_EXPORT YDebugStream yzDeepDebug( const char * area = "" );

/** Convenient function to build an deep debug stream. */
YZIS_EXPORT YDebugStream yzDebug( const char * area = "" );

/** Convenient function to build a warning stream. */
YZIS_EXPORT YDebugStream yzWarning( const char * area = "" );

/** Convenient function to build an error stream. */
YZIS_EXPORT YDebugStream yzError( const char * area = "" );

/** Convenient function to build an fatal stream. */
YZIS_EXPORT YDebugStream yzFatal( const char * area = "" );


//! \name Assertion macros
//! @{

/** Truth assertion with custom message.
  *
  * The assertion will automatically include file:line information and
  * copy of the assertion.
  *
  * If the assertion fails, an error is displayed in the empty area.
  */
#define YASSERT_MSG( assertion, msg ) { if (! (assertion) ) { yzError() << QString("%1:%2 assertion '%3' failed : %4\n").arg(__FILE__).arg( __LINE__).arg(#assertion).arg( msg ); } }

/** Truth assertion with empty message.
  *
  * The assertion will automatically include file:line information and
  * copy of the assertion.
  *
  * If the assertion fails, an error is displayed in the empty area.
  */
#define YASSERT( assertion ) YASSERT_MSG( assertion, "" )


/** Equality assertion
  *
  * The assertion will automatically include file:line information and
  * copy of the assertion.
  *
  * If the assertion fails, an error is displayed in the empty area.
  */
#define YASSERT_EQUALS( a, b ) { if (a != b) { yzError() << QString("%1:%2 - %3 == %4 failed : '%5' != '%6'\n").arg(__FILE__).arg( __LINE__).arg(#a).arg(#b).arg(a).arg(b); } }

//! @}

//! \name Convenient macros for debugging
//! @{
/** qPrintable is a Qt function that can turn any Qt type into a const char *.
  *
  * qp() provides a shortcut for qPrintable.
  */
#define qp(s)   qPrintable(s)

/** Returns a string composed of the current function name and the current
  * line number. Quite convenienent to quickly trace some code execution:
  *
  * Example:
  * \code
  * void f1()
  * {
  *     dbg() << HERE() << endl;
  *     // more stuff
  * }
  *
  * void f1( int a )
  * {
  *     dbg() << HERE() << endl;
  *     // more stuff with a
  * }
  * \endcode
  *
  * In the example, the function name will be the same, but the line number
  * will help to distinguish between the two calls.
  */
#define HERE() qp(QString("%1:%2 ").arg(__PRETTY_FUNCTION__).arg(__LINE__))

/** Returns a string composed of the filename, a colon and the current line
  * number of the execution. Quite convenient when debugging:
  *
  * Example:
  * \code
  * void f1() 
  * {
  *     dbg() << LOCATION() << endl;
  * }
  * \endcode
  *
  * In practice, HERE() is preferred because knowing the function name
  * is more precise information than just knowing the file.
  */
#define LOCATION() qp(QString("%1:%2 ").arg(__FILE__).arg(__LINE__))

//! @}

#endif /* YZ_DEBUG_H */
