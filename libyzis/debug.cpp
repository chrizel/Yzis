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
 */


#include "debug.h"
#include <qstringlist.h>
#include <qfile.h>
#include <qregexp.h>
#include <ctype.h>
#include <unistd.h>

YZDebugBackend * YZDebugBackend::_instance = NULL;

YZDebugBackend::YZDebugBackend()
{
	_output = fopen("/tmp/yzisdebug-" + QString(cuserid(NULL)) + ".log", "w" );
	init();
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

void YZDebugBackend::setDebugOutput( const char * fileName )
{
	setDebugOutput( fopen( fileName, "w" ) );
}

void YZDebugBackend::flush( int level, const char * area, const char * data )
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
}

void YZDebugBackend::parseRcfile(const char * filename)
{
	flush( YZ_DEBUG_LEVEL,"YZDebugBackend", QString("parseRcfile(%1)\n").arg(filename).latin1() );
	QFile f( filename );
	if (f.open( IO_ReadOnly ) == false) return;
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
		if (enableRe.search(l) == 0) {
			area = enableRe.cap(1);
			//flush( YZ_DEBUG_LEVEL, "YZDebugBackend", QString("enable '%1'\n").arg(area).latin1() );
			enableDebugArea(area, true );
		} else if (disableRe.search(l) == 0) {
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

YZDebugStream& YZDebugStream::operator << (const QCString& string) {
	*this << string.data();
	return *this;
}

YZDebugStream& YZDebugStream::operator << (const QString& string) {
	output+=string;
	if ( output.at( output.length() - 1 ) == '\n' )
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
	YZDebugBackend::instance()->flush(level, area, output.local8Bit().data());
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

