/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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


#include <qtranslator.h>
#include <qtextcodec.h>
#include <qapp.h>
#include "libyzis/translator.h"
#include "testBuffer.h"
#include "PhilTestRunner.h"

int main(int argc, char ** argv)
{
	int ret;
	bool doWait = false;
#ifdef WIN32
	doWait = true;
#endif

	QApplication app(  argc, argv );
	QTranslator qt(  0 );
	qt.load(  QString(  "qt_" ) + QTextCodec::locale(), "." );
	app.installTranslator(  &qt );
	QTranslator myapp(  0 );
	myapp.load(  QString(  "yzis_" ) + QTextCodec::locale(), QString( PREFIX ) + "/share/yzis/locale/" );
	app.installTranslator(  &myapp );
	

	PhilTestRunner runner;
	runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );

	if (argc > 1) {
		for( int i=1; i<argc; i++) {
			ret = runner.run( argv[i], doWait, true, true ) && ret;
		}
	} else {
		runner.run( "", doWait, true, true );
	}
	return 0;
}




