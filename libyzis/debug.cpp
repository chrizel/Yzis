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
 * This file was mostly inspired from the kdelibs kdDebug class distributed under LGPL by
 * by the KDE project.
 * Here are the corresponding copyrights owner :
 * 1997 Matthias Kalle Dalheimer ( kalle@kde.org )
 * 2000-2002 Stephan Kulow ( coolo@kde.org )
 */

/* Yzis */
#include "debug.h"
#include "portability.h"

/* Std */
#include <ctype.h>
#include <stdarg.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Qt */
#include <QTextStream>
#include <QRegExp>
#include <QStringList>
#include <QFile>

#ifdef YZIS_WIN32_GCC 
// to use OutputDebugString
#include <windows.h>
#endif

#define dbg()    yzDebug("YDebugBackend")
#define err()    yzError("YDebugBackend")

// Choose one of the two above to enable printf debugging or not
// printf debugging is the only way to debug this file
// #define DBG_SELF( s )    s
#define DBG_SELF( s )

// #define DBG_FLUSH( s )    s
#define DBG_FLUSH( s )

// #define DBG_AREALEVEL( s )    s
#define DBG_AREALEVEL( s )

YDebugBackend * YDebugBackend::me = NULL;

YDebugBackend::YDebugBackend()
{
    qDebug("YDebugBackend::YDebugBackend() constructor");
    _output = NULL;
    _outputFname = "";
}

YDebugBackend::YDebugBackend( YDebugBackend & )
        : boost::noncopyable()
{
    qFatal("YDebugBackend copy constructor used %s", HERE());
}

YDebugBackend & YDebugBackend::operator=( YDebugBackend & )
{
    qFatal("YDebugBackend operator = used %s", HERE());
    return *this;
}

YDebugBackend::~YDebugBackend()
{
    dbg() << "~YDebugBackend()" << endl;
    if ( _output != NULL ) {
        fclose( _output );
    }
}

void YDebugBackend::init()
{
    clearArea();
    _levelByName["deepdebug"] = YZ_DEEPDEBUG_LEVEL;
    _levelByName["debug"] = YZ_DEBUG_LEVEL;
    _levelByName["warning"] = YZ_WARNING_LEVEL;
    _levelByName["error"] = YZ_ERROR_LEVEL;
    _levelByName["fatal"] = YZ_FATAL_LEVEL;

    _nameByLevel[YZ_DEEPDEBUG_LEVEL] = "deepdebug";
    _nameByLevel[YZ_DEBUG_LEVEL] = "debug";
    _nameByLevel[YZ_WARNING_LEVEL] = "warning";
    _nameByLevel[YZ_ERROR_LEVEL] = "error";
    _nameByLevel[YZ_FATAL_LEVEL] = "fatal";

#ifdef DEBUG
    _level = YZ_DEBUG_LEVEL;
#else
    _level = YZ_WARNING_LEVEL;
#endif

#ifndef YZIS_WIN32_GCC
    setDebugOutput( "/tmp/yzisdebug-" + QString(getpwuid(geteuid())->pw_name) + ".log" );
#else
    setDebugOutput( "/tmp/yzisdebug.log" );
#endif

    // our message handler does not manage to display all messages. So,
    // it is better left off disabled at the moment. The last one gets lost
    // and the last one is the most interesting one.

    qInstallMsgHandler( yzisMsgHandler );

}

YDebugBackend * YDebugBackend::self()
{
    DBG_SELF( printf("YDebugBackend::self() me = %p\n", YDebugBackend::me ); )
    if (YDebugBackend::me == NULL) {
        me = new YDebugBackend();
        DBG_SELF( printf("YDebugBackend::self() me set to %p\n", me ); )
        me->init();
    }
    return me;
}

void YDebugBackend::setDebugOutput( FILE * file )
{
    if (file == NULL) {
        flush( YZ_WARNING_LEVEL, 0, "YDebugBackend: setting output to a NULL file descriptor\n" );
        return ;
    }
    setvbuf( file, NULL, _IONBF, 0 ); // disable buffering
    _output = file;
}

void YDebugBackend::setDebugOutput( const QString& fileName )
{
    if ( _output != NULL) {
        dbg().sprintf( "setDebugOutput( %s )", qp( fileName ) );

        if (_output != stdout && _output != stderr ) {
            fclose( _output );
        }
        _output = NULL;
        _outputFname = "";
    }

    _outputFname = fileName;

    if (fileName == "stdout") {
        setDebugOutput( stdout );
        dbg() << "Debug output set to stdout" << endl;
        return ;
    } else if (fileName == "stderr") {
        setDebugOutput( stderr );
        dbg() << "Debug output set to stderr" << endl;
        return ;
    }

    if ( QFile::exists( fileName ) ) {
        QFile::remove ( fileName );
    }

    FILE * f = fopen( fileName.toLocal8Bit(), "w" );
    setDebugOutput( f );
    dbg().sprintf( "_output set to file %s: FILE * = %p\n", qp(fileName), f );

#ifndef YZIS_WIN32_GCC
    struct stat buf;
    int i = lstat( fileName.toLocal8Bit(), &buf );
    if ( i != -1 && S_ISREG( buf.st_mode ) && !S_ISLNK( buf.st_mode ) && buf.st_uid == geteuid() )
        chmod( fileName.toLocal8Bit(), S_IRUSR | S_IWUSR );
#else
    if ( true )
        ;
#endif
    else {
        fclose( _output );
        _output = NULL;
        _outputFname = "";
    }
}

void YDebugBackend::flush( int level, const QString& area, const char * data )
{
    DBG_FLUSH( printf("flush(): output=%p arealevel=%d, level=%d, area=%s, data='%s'\n", _output, areaLevel(area), level, qp(area), data ); )
    DBG_FLUSH( printf("flush(): this=%p _output=%p stdout=%p stderr=%p \n", this, _output, stdout, stderr ); )
    if (level < areaLevel(area)) {
        DBG_FLUSH( printf("flush(): no flush!\n"); )
        return ;
    }
    Q_ASSERT_X( _output != NULL, HERE(), "Output stream should not be NULL" );
    if (_output == NULL) return ;
    fprintf( _output, "%s\n", data ); // data never ends with \n
    fflush( _output );
#ifdef YZIS_WIN32_GCC
    _flushall();
    OutputDebugString( data );
#endif
    DBG_FLUSH( printf("flush(): done\n"); )
}

void YDebugBackend::parseRcfile(const char * filename)
{
    dbg().sprintf("parseRcfile(%s)\n", filename );
    QFile f( filename );
    if (f.open( QIODevice::ReadOnly ) == false) {
        err() << "Could not open rcfile '" << filename << "'" << endl;
        return ;
    }
    QTextStream ts(&f);

    /* One can imagine more control of the output, like whether to use a file
     * or not, which is the default debug level.
     */
    QRegExp lineRe( "^\\s*(\\w+)\\s*:\\s*(\\w+)\\s*$" );

    QString l, area;
    while ( ts.atEnd() == false) {
        l = ts.readLine();
        dbg() << "l='" << l << "'" << endl;
        if (l.isEmpty()) continue;
        if (l.at(0) == (char) '#' ) continue;
        if (!lineRe.exactMatch(l)) continue;
        QString keyword = lineRe.cap(1).trimmed();
        QString action = lineRe.cap(2).trimmed();
        if (keyword == "output") {
            setDebugOutput( action );
        } else if (keyword == "level") {
            if (! _levelByName.contains(action)) {
                err() << "Unknown debug level in " << filename << ": " << action << endl;
            } else {
                setDebugLevel( _levelByName[action] );
            }
        } else { // keyword is area
            if (! _levelByName.contains(action)) {
                err() << "Unknown debug level in " << filename << "for area " << keyword << ": " << action << endl;
            } else {
                setAreaLevel( keyword, _levelByName[action] );
            }
        }
    }
    f.close();
    dbg().sprintf("Parsing done for %s\n", filename );
    dbg() << toString();
}


void YDebugBackend::parseArgv(int argc, char ** argv)
{
    QStringList argvSl;
    for ( int i = 0; i < argc; i++) {
        argvSl << argv[i];
    }
    parseArgv( argvSl );
}

void YDebugBackend::parseArgv( QStringList & argv )
{
    QRegExp reLevelOpt("--level=(\\w+)");
    QRegExp reAreaOpt("--area-level=(\\w+),(\\w+)");
    QRegExp reDebugOutput("--debug-output=(\\w+)");
    dbg() << "argv='" << argv << "'" << endl;
    for (int i = argv.count() - 1; i >= 1; i--) {
        dbg() << "argv[i]='" << argv[i] << "'" << endl;
        QString myargv = QString(argv[i]).trimmed();
        if (reLevelOpt.exactMatch(myargv)) {
            QString sLevel = reLevelOpt.cap(1);
            dbg() << "sLevel='" << sLevel << "'" << endl;
            argv.removeAt( i );
            if ( _levelByName.contains(sLevel) == false) {
                err().sprintf("global debug level unrecognised: %s", qp(sLevel));
            } else {
                setDebugLevel( _levelByName[sLevel] );
            }
        } else if (reAreaOpt.exactMatch(myargv)) {
            QString sArea = reAreaOpt.cap(1);
            QString sLevel = reAreaOpt.cap(2);
            dbg() << "sLevel='" << sLevel << "'" << endl;
            dbg() << "sArea='" << sArea << "'" << endl;
            argv.removeAt( i );
            if ( _levelByName.contains(sLevel) == false) {
                yzError("YDebugBackend").sprintf("debug level unrecognised for area %s: %s", qp(sArea), qp(sLevel));
            } else {
                setAreaLevel( sArea, _levelByName[sLevel] );
            }
        } else if (reDebugOutput.exactMatch(myargv)) {
            QString sFilename = reDebugOutput.cap(1);
            dbg() << "sFilename='" << sFilename << "'" << endl;
            argv.removeAt( i );
            setDebugOutput( sFilename );
        }
    }
    dbg() << toString();
}
void YDebugBackend::setAreaLevel( const QString& area, int level )
{
    dbg() << "setAreaLevel(" << area << ", " << _nameByLevel[level] << ")" << endl;
    _areaLevel[area] = level;
}

int YDebugBackend::areaLevel( const QString& area ) const
{
    // you can not debug here with dbg(), else there is a recursive loop
    QString found;
    int areaLevel = debugLevel();
    foreach( QString itArea, _areaLevel.keys() ) {
        DBG_AREALEVEL( printf("areaLevel(): Checking area %s with debug area %s\n", qp(area), qp(itArea) ); )
        if (area.startsWith( itArea )
                && found.length() < itArea.length() ) {
            // found a better matching area
            found = itArea;
            areaLevel = _areaLevel[itArea];
            DBG_AREALEVEL( printf("areaLevel(): Found new area level for %s, matching with %s: %d\n", qp(area), qp(itArea), areaLevel ); )
        }
    }
    DBG_AREALEVEL( printf("areaLevel(): returning %d\n", areaLevel ); )
    return areaLevel;
}


QString YDebugBackend::toString()
{
    QString s;

    s += QString("YDebugBackend content:\n");
    s += QString("level: %1\n").arg(_nameByLevel[ debugLevel() ]);
    s += QString("output: %1\n").arg(_outputFname);
    foreach( QString area, _areaLevel.keys() ) {
        s += QString("%1:%2\n").arg(area).arg(_nameByLevel[_areaLevel.value(area)]);
    }
    return s;
}

void YDebugBackend::yzisMsgHandler( QtMsgType msgType, const char * msg )
{
    // It does not seem to be working. We do not display the last message
    // send by Qt when a qFatal() or Q_ASSERT() failure occurs.
    // So, I recommend to disable it.

    /*
    DBG_SELF( printf("========== Qt yzisMsgHandler called!\n"); )
    DBG_SELF( printf("========== type %d\n", (int) msgType ); )
    DBG_SELF( printf("========== msg %s\n", msg ); )
    dbg() << HERE() << msg << endl;
    */
    switch (msgType) {
    case QtDebugMsg:
        yzDebug("Qt") << msg << endl;
        break;
    case QtWarningMsg:
        yzWarning("Qt") << msg << endl;
        break;
    case QtCriticalMsg:
        yzError("Qt") << msg << endl;
        break;
    case QtFatalMsg:
        yzFatal("Qt") << msg << endl;
        break;
    default:
        yzDebug("Qt") << msg << endl;
        break;
    }
}

void * yzmalloc( size_t tSize )
{
    return malloc( tSize );
}

void yzfree( void * p )
{
    free(p);
}

void * YDebugBackend::operator new( size_t tSize )
{
    DBG_SELF( printf("YDebugBackend::new( %ld )\n", long(tSize)); )
    return yzmalloc( tSize );
}

void YDebugBackend::operator delete( void *p )
{
    DBG_SELF( printf("YDebugBackend::delete %p\n", p ); )
    yzfree(p);
}


// ================================================================
//
//                          YDebugStream
//
// ================================================================

YDebugStream::YDebugStream( const char * _area, int _level )
{
    area = _area;
    level = _level;
    if (strlen(_area)) output = QString(_area) + ':';
}

YDebugStream::~YDebugStream()
{
    if ( !output.isEmpty() )
        *this << "\n"; //flush
}

void YDebugStream::sprintf( const char * fmt, ... )
{
    static char buf[1024];

    va_list ap;
    va_start( ap, fmt );
    vsnprintf( buf, 1024, fmt, ap );
    va_end(ap);
    buf[1023] = '\0';
    output += buf;
    flush();
}

YDebugStream& YDebugStream::operator << (const QString& string)
{
    output += string;
    if ( output.endsWith("\n") ) {
        flush();
    }
    return *this;
}

YDebugStream& YDebugStream::operator << (const char* string)
{
    output += QString::fromUtf8( string );
    if ( output.at( output.length() - 1 ) == '\n' ) {
        flush();
    } else {
        // output += " ";
    }
    return *this;
}

YDebugStream& YDebugStream::operator << (const QStringList& string)
{
    *this << "(";
    *this << string.join( "," );
    *this << ") ";
    return *this;
}

YDebugStream& YDebugStream::operator << (bool i)
{
    output += QString::fromLatin1(i ? "true " : "false ");
    return *this;
}

YDebugStream& YDebugStream::operator << (char i)
{
    if ( isprint( i ) ) {
        output += "\\x" + QString::number(static_cast<uint>( i ) + 0x100, 16 ).right( 2 );
    } else {
        output += i;
    }
    if ( i == '\n' ) flush();
    else output += ' ';
    return *this;
}

YDebugStream& YDebugStream::operator << (unsigned char i)
{
    return operator<<(static_cast<char>(i));
}

YDebugStream& YDebugStream::operator << (unsigned short i)
{
    QString tmp;
    tmp.setNum( i );
    output += tmp + ' ';
    return *this;
}

YDebugStream& YDebugStream::operator << (short i)
{
    QString tmp;
    tmp.setNum( i );
    output += tmp + ' ';
    return *this;
}

YDebugStream& YDebugStream::operator << (unsigned int i)
{
    QString tmp;
    tmp.setNum( i );
    output += tmp + ' ';
    return *this;
}

YDebugStream& YDebugStream::operator << (int i)
{
    QString tmp;
    tmp.setNum( i );
    output += tmp + ' ';
    return *this;
}

YDebugStream& YDebugStream::operator << (unsigned long i)
{
    QString tmp;
    tmp.setNum( i );
    output += tmp + ' ';
    return *this;
}

YDebugStream& YDebugStream::operator << (long i)
{
    QString tmp;
    tmp.setNum( i );
    output += tmp + ' ';
    return *this;
}

YDebugStream& YDebugStream::operator << (double d)
{
    QString tmp;
    tmp.setNum( d );
    output += tmp + ' ';
    return *this;
}

void YDebugStream::flush()
{
    if ( output.right(1) == "\n" ) output = output.left( output.length() - 1 );
    if ( output.isEmpty() ) return ;
    YDebugBackend::self()->flush(level, area, output.toUtf8());
    output.clear();
}

YDebugStream yzDeepDebug( const char * area )
{
    return YDebugStream( area, YZ_DEEPDEBUG_LEVEL );
}
YDebugStream yzDebug( const char * area )
{
    return YDebugStream( area, YZ_DEBUG_LEVEL );
}
YDebugStream yzWarning( const char * area )
{
    return YDebugStream( area, YZ_WARNING_LEVEL );
}
YDebugStream yzError( const char * area )
{
    return YDebugStream( area, YZ_ERROR_LEVEL );
}
YDebugStream yzFatal( const char * area )
{
    return YDebugStream( area, YZ_FATAL_LEVEL );
}

