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
	QObject::connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );

	QTranslator qt(  0 );
	qt.load(  QString(  "qt_" ) + QTextCodec::locale(), "." );
	app.installTranslator(  &qt );
	QTranslator myapp(  0 );
//	myapp.load(  QString(  "yzis_" ) + QTextCodec::locale(), QString( PREFIX ) + "/share/yzis/locale/" );
	app.installTranslator(  &myapp );

	(new QYZSession())->show();
	QString initialSendKeys;

    // handle command line options
//	YZSession::mOptions.setGroup("Global");
	bool splash = YZSession::getBoolOption("blocksplash");

	/*
	 * Open buffers
	 */
	bool hasatleastone = false;
	QString s;

	/** read options **/

	for ( int i=1; i<argc; i++ ) {
		if ( '-' != argv[i][0] ) {
			hasatleastone = true;
			yzDebug(QYZIS)<< "qyzis : opening file " << argv[i]<<endl;
			QYZSession::me->createBuffer(argv[ i ]);
		} else {
			s = QString( argv[i] );
			if (s == "-h" || s == "--help") {
				printf("QYzis, yzis port of Yzis - http://www.yzis.org\n"
					VERSION_CHAR_LONG " " VERSION_CHAR_DATE );
				printf("\nUsage : %s [--help|-h] [--version|-v] [filename1 [filename2] .... ]\n", argv[0]);
				exit(0);
			} else if (s == "-v" || s == "--version") {
				printf("Nyzis, ncurses part of Yzis - http://www.yzis.org\n"
					VERSION_CHAR_LONG " " VERSION_CHAR_DATE "\n");
				exit(0);
			} else if (s == "-c") {
				QString optArg;
				YZSession::setBoolOption("blocksplash", false);
				if (s.length() > 2) optArg = argv[i]+2;
				else if (i < argc-1) optArg = argv[i+1];
				initialSendKeys = optArg;
			} else {
				printf("Unrecognised option: %s\n", argv[i] );
				exit(-1);
			}
		}
	}

	if ( !hasatleastone ) {
		QYZSession::me->createBuffer();
		YZView* cView = YZSession::me->currentView();
		cView->displayIntro();
	}

	if (initialSendKeys.length()) {
		YZSession::me->sendMultipleKeys( initialSendKeys );
		YZSession::setBoolOption("blocksplash", splash);
	}

	return app.exec();
}

