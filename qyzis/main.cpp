/*
    Copyright (c) 2004 Philippe Fremy <phil@freehackers.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/**
 * $Id: main.cpp 1272 2004-10-16 22:41:00Z mikmak $
 */

#include <qapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include "libyzis/portability.h"
#include "libyzis/session.h"
#include "libyzis/view.h"
#include "qyzview.h"
#include "qyzsession.h"
#include "qyzbuffer.h"


int main(int argc, char **argv) {
	QApplication app( argc, argv );

	QTranslator qt(  0 );
	qt.load(  QString(  "qt_" ) + QTextCodec::locale(), "." );
	app.installTranslator(  &qt );
	QTranslator myapp(  0 );
//	myapp.load(  QString(  "yzis_" ) + QTextCodec::locale(), QString( PREFIX ) + "/share/yzis/locale/" );
	app.installTranslator(  &myapp );

	QYZSession session;
	QYZBuffer buffer(&session);
	QYZView * w = new QYZView( &buffer, &session, 50 );

	return app.exec();
}
