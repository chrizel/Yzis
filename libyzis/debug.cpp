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


#include "portability.h"
#include "debug.h"
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <QTextStream>
#include <QRegExp>
#include <QStringList>
#include <QFile>

#ifdef YZIS_WIN32_GCC
// to use OutputDebugString
#include <windows.h>
#endif

#define dbg()    yzDebug("YZDebugBackend")
#define err()    yzError("YZDebugBackend")


YZDebugBackend * YZDebugBackend::me = NULL;

YZDebugBackend::YZDebugBackend()
{
	_output = NULL;

#ifndef YZIS_WIN32_GCC
	setDebugOutput( "/tmp/yzisdebug-" + QString(getpwuid(geteuid())->pw_name) + ".log" );
#else
	setDebugOutput( "/tmp/yzisdebug.log" );
#endif
	init();
}

YZDebugBackend::~YZDebugBackend() {
	if ( _output != NULL )
		fclose( _output );
}

void YZDebugBackend::init()
{
    // our message handler does not manage to display all messages. So,
    // it is better left off disabled at the moment. The last one gets lost
    // and the last one is the most interesting one.

    // qInstallMsgHandler( yzisMsgHandler );

#ifdef DEBUG
	_level = YZ_DEBUG_LEVEL;
#else
	_level = YZ_WARNING_LEVEL;
#endif

	clearArea();
    _levelByName["debug"] = YZ_DEBUG_LEVEL;
    _levelByName["warning"] = YZ_WARNING_LEVEL;
    _levelByName["error"] = YZ_ERROR_LEVEL;
    _levelByName["fatal"] = YZ_FATAL_LEVEL;

    _nameByLevel[YZ_DEBUG_LEVEL] = "debug";
    _nameByLevel[YZ_WARNING_LEVEL] = "warning";
    _nameByLevel[YZ_ERROR_LEVEL] = "error";
    _nameByLevel[YZ_FATAL_LEVEL] = "fatal";
}

YZDebugBackend * YZDebugBackend::self()
{
	if (me == NULL) {
		me = new YZDebugBackend();
	}
	return me;
}

void YZDebugBackend::setDebugOutput( FILE * file )
{
	if (file == NULL) {
		flush( YZ_WARNING_LEVEL, 0, "YZDebugBackend: setting output to a NULL file descriptor\n" );
		return;
	}
    setvbuf( file, NULL, _IONBF, 0 ); // disable buffering
	_output = file;
}

void YZDebugBackend::setDebugOutput( const QString& fileName )
{
	if ( _output != NULL) {
        if (_output != stdout && _output != stderr ) {
            fclose( _output );
        }
		_output = NULL;
	}

    if (fileName == "stdout") {
        setDebugOutput( stdout );
        return;
    } else if (fileName == "stderr") {
        setDebugOutput( stderr );
        return;
    }

	if ( QFile::exists( fileName ) )
		QFile::remove ( fileName );

    FILE * f = fopen( fileName.toLocal8Bit(), "w" );
	setDebugOutput( f );

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
	}
}

void YZDebugBackend::flush( int level, const QString& area, const char * data )
{
    //printf("flush: arealevel=%d, level=%d, area=%s, data=%s\n", areaLevel(area), level, qp(area), data );
	if (level < areaLevel(area)) return;
	fprintf( _output, "%s", data );
	fflush( _output );
#ifdef YZIS_WIN32_GCC
    _flushall();
    OutputDebugString( data );
#endif
}

void YZDebugBackend::parseRcfile(const char * filename)
{
    dbg().sprintf("parseRcfile(%s)\n", filename );
    QFile f( filename );
    if (f.open( QIODevice::ReadOnly ) == false) {
        err() << "Could not open rcfile '" << filename << "'" << endl;
        return;
    }
    QTextStream ts(&f);

    /* One can imagine more control of the output, like whether to use a file
     * or not, which is the default debug level.
     */
    QRegExp lineRe( "^\\s*(\\w+)\\s*:\\s*(\\w+)\\s*$" );

    QString l, area;
    while( ts.atEnd() == false) {
        l = ts.readLine();
        dbg() << "l='" << l << "'" << endl;
        if (!lineRe.exactMatch(l)) continue;
        QString keyword = lineRe.cap(1).trimmed().toLower();
        QString action  = lineRe.cap(2).trimmed().toLower();
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
                err() << "Unknown debug level in " << filename << "for area " << keyword << " : " << action << endl; 
            } else {
                setAreaLevel( keyword, _levelByName[action] );
            }
        }
    }
}


void YZDebugBackend::parseArgv(int argc, char ** argv)
{
    QStringList argvSl;
    for( int i=0; i<argc; i++) {
        argvSl << argv[i];
    }
    parseArgv( argvSl );
}

void YZDebugBackend::parseArgv( QStringList & argv )
{
    QRegExp reLevelOpt("--level=(\\w+)");
    QRegExp reAreaOpt("--area-level=(\\w+),(\\w+)");
    QRegExp reDebugOutput("--debug-output=(\\w+)");
    dbg() << "argv='" << argv << "'" << endl;  
    for(int i=argv.count()-1; i>=1; i--) {
        dbg() << "argv[i]='" << argv[i] << "'" << endl;  
        QString myargv=QString(argv[i]).trimmed();
        dbg() << "myargv='" << myargv << "'" << endl;  
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
            QString sArea  = reAreaOpt.cap(1);
            QString sLevel = reAreaOpt.cap(2);
            dbg() << "sLevel='" << sLevel << "'" << endl;
            dbg() << "sArea='" << sArea << "'" << endl;
            argv.removeAt( i );
            if ( _levelByName.contains(sLevel) == false) {
                yzError("YZDebugBackend").sprintf("debug level unrecognised for area %s: %s", qp(sArea), qp(sLevel));
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
}
void YZDebugBackend::setAreaLevel( const QString& area, int level ) 
{
    _areaLevel[area] = level;
}

int YZDebugBackend::areaLevel( const QString& area ) const
{
    // you can not debug here with dbg(), else there is a recursive loop
    QString found;
    int areaLevel = debugLevel();
    foreach( QString itArea, _areaLevel.keys() ) {
        if (area.startsWith( itArea )
            && found.length() < itArea.length() ) {
            // found a better matching area
            found = itArea;
            areaLevel = _areaLevel[itArea];
        }
    }
    return areaLevel;
}


QString YZDebugBackend::toString()
{
    QString s;

    
    s += QString("Debug level: %1\n").arg(_nameByLevel[ debugLevel() ]);
    foreach( QString area, _areaLevel.keys() ) {
        s += QString("%1:%2\n").arg(area).arg(_nameByLevel[_areaLevel.value(area)]);
    }
    return s;
}

void YZDebugBackend::yzisMsgHandler( QtMsgType msgType, const char * msg )
{
    // It does not seem to be working. We do not display the last message
    // send by Qt when a qFatal() or Q_ASSERT() failure occurs.
    // So, I recommend to disable it.

    /*
    printf("========== Qt yzisMsgHandler called!\n");
    printf("========== type %d\n", (int) msgType );
    printf("========== msg %s\n", msg );
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

// ================================================================
//                          
//                          YZDebugStream
//                          
// ================================================================

YZDebugStream::YZDebugStream( const char * _area, int _level ) {
	area = _area;
	level = _level;
    if (strlen(_area)) output=QString(_area)+":";
}

YZDebugStream::~YZDebugStream() {
	if ( !output.isEmpty() )
		*this << "\n"; //flush
}

void YZDebugStream::sprintf( const char * fmt, ... )
{
    static char buf[256];

    va_list ap;
    va_start( ap, fmt );
    vsnprintf( buf, 256, fmt, ap );
    va_end(ap);
    buf[255] = '\0';
    output += buf;
    if (output.right(1) != "\n") output += '\n';
    flush();
}

YZDebugStream& YZDebugStream::operator << (const QString& string) {
	output+=string;
	if ( output.endsWith("\n") )
		flush();
	return *this;
}

YZDebugStream& YZDebugStream::operator << (const QStringList& string) {
	*this << "(";
	*this << string.join( "," );
	*this << ")";
	return *this;
}

YZDebugStream& YZDebugStream::operator << (const char* string) {
	output+=QString::fromUtf8( string );
//	if ( output.at( output.length() - 1 ) == '\n' )
		flush();
	return *this;
}

YZDebugStream& YZDebugStream::operator << (bool i) {
	output+=QString::fromLatin1(i ? "true" : "false");
	return *this;
}

YZDebugStream& YZDebugStream::operator << (char i) {
	if ( isprint( i ) )
		output+= "\\x" + QString::number(static_cast<uint>( i )+0x100,16 ).right( 2 );
	else
		output+= i;
	if ( i == '\n' ) flush();
	return *this;
}

YZDebugStream& YZDebugStream::operator << (unsigned char i) {
	return operator<<(static_cast<char>(i));
}

YZDebugStream& YZDebugStream::operator << (unsigned short i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (short i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (unsigned int i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (int i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (unsigned long i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (long i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (double d) {
	QString tmp;
	tmp.setNum( d );
	output+=tmp;
	return *this;
}

void YZDebugStream::flush() {
	if ( output.isEmpty() ) return;
	YZDebugBackend::self()->flush(level, area, output.toUtf8());
	output=QString::null;
}

YZDebugStream yzDebug( const char * area ) {
	return YZDebugStream( area, YZ_DEBUG_LEVEL );
}
YZDebugStream yzWarning( const char * area ) {
	return YZDebugStream( area, YZ_WARNING_LEVEL );
}
YZDebugStream yzError( const char * area ) {
	return YZDebugStream( area, YZ_ERROR_LEVEL );
}
YZDebugStream yzFatal( const char * area ) {
	return YZDebugStream( area, YZ_FATAL_LEVEL );
}

