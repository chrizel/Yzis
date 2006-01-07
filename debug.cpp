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
#include <sys/stat.h>
#include <sys/types.h>
#include <QTextStream>
#include <QRegExp>
#include <QStringList>

YZDebugBackend * YZDebugBackend::_instance = NULL;

YZDebugBackend::YZDebugBackend()
{
	_output = NULL;
#ifndef YZIS_WIN32_MSVC
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
	_level = 0;
	clear();
	parseRcfile( FILENAME_DEBUGRC );
}

YZDebugBackend * YZDebugBackend::instance()
{
	if (_instance == NULL) {
		_instance = new YZDebugBackend();
	}
	return _instance;
}

void YZDebugBackend::setDebugOutput( FILE * file )
{
	if (file == NULL) {
		flush( YZ_WARNING_LEVEL, 0, "YZDebugBackend: setting output to a NULL file descriptor\n" );
		return;
	}
	_output = file;
}

void YZDebugBackend::setDebugOutput( const QString& fileName )
{
	if ( _output != NULL ) {
		fclose( _output );
		_output = NULL;
	}

	if ( QFile::exists( fileName ) )
		QFile::remove ( fileName );
	struct stat buf;
#if QT_VERSION < 0x040000
	setDebugOutput( fopen( fileName.local8Bit(), "w" ) );
	int i = lstat( fileName.local8Bit(), &buf );
	if ( i != -1 && S_ISREG( buf.st_mode ) && !S_ISLNK( buf.st_mode ) && buf.st_uid == geteuid() ) 
		chmod( fileName.local8Bit(), S_IRUSR | S_IWUSR );
	else {
		fclose( _output );
		_output = NULL;
	}
#else
	setDebugOutput( fopen( fileName.toLocal8Bit(), "w" ) );
#ifndef YZIS_WIN32_MSVC
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
#endif
}

void YZDebugBackend::flush( int level, const QString& area, const char * data )
{
	if (level < _level) return;
	if (isAreaEnabled( area ) == false) return;

	fprintf( _output, "%s", data );
	/*
	if (data[strlen(data)-1] != '\n') {
		fprintf( _output, "\n" );
	}
	*/
	fflush( _output );
#ifdef YZIS_WIN32_MSVC
	OutputDebugStringA( data );
#endif
}

void YZDebugBackend::parseRcfile(const char * filename)
{
#if QT_VERSION < 0x040000
	flush( YZ_DEBUG_LEVEL,"YZDebugBackend", QString("parseRcfile(%1)\n").arg(filename).latin1() );
	QFile f( filename );
	if (f.open( IO_ReadOnly ) == false) return;
#else
	flush( YZ_DEBUG_LEVEL,"YZDebugBackend", QString("parseRcfile(%1)\n").arg(filename).toLatin1() );
	QFile f( filename );
	if (f.open( QIODevice::ReadOnly ) == false) return;
#endif
	QTextStream ts(&f);

	/* One can imagine more control of the output, like whether to use a file
	 * or not, which is the default debug level.
	 */
	QRegExp enableRe("enable:(\\w+)");
	QRegExp disableRe("disable:(\\w+)");
	QString l, area;
	while( ts.atEnd() == false) {
		l = ts.readLine();
		//flush( YZ_DEBUG_LEVEL, "YZDebugBackend", QString("line '%1'\n").arg(l).latin1() );
#if QT_VERSION < 0x040000
		if (enableRe.search(l) == 0) {
#else
		if (enableRe.indexIn(l) == 0) {
#endif
			area = enableRe.cap(1);
			//flush( YZ_DEBUG_LEVEL, "YZDebugBackend", QString("enable '%1'\n").arg(area).latin1() );
			enableDebugArea(area, true );
#if QT_VERSION < 0x040000
		} else if (disableRe.search(l) == 0) {
#else
		} else if (disableRe.indexIn(l) == 0) {
#endif
			area = disableRe.cap(1);
			//flush( YZ_DEBUG_LEVEL, "YZDebugBackend", QString("disable '%1'\n").arg(area).latin1() );
			enableDebugArea(area, false );
		}
	}
}


YZDebugStream::YZDebugStream( const char * _area, int _level ) {
	area = _area;
	level = _level;
}

YZDebugStream::~YZDebugStream() {
	if ( !output.isEmpty() )
		*this << "\n"; //flush
}

#if QT_VERSION < 0x040000
YZDebugStream& YZDebugStream::operator << (const QCString& string) {
	*this << string.data();
	return *this;
}
#endif

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
#if QT_VERSION < 0x040000
	YZDebugBackend::instance()->flush(level, area, output.local8Bit().data());
#else
	YZDebugBackend::instance()->flush(level, area, output.toUtf8());
#endif
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

