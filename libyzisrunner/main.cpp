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

#include "TSession.h"

#ifdef Q_WS_X11
/* X11 */
#include <X11/Xlib.h>
#endif

int main(int argc, char **argv) {
#ifdef Q_WS_X11
	bool useGUI = getenv(  "DISPLAY" ) != 0;
	if (useGUI) {
		Display *d = XOpenDisplay(NULL);
		if (d == NULL) //woups we failed to connect to X
			useGUI = false; // so don't try again later ;)
	}
#else
	bool useGUI = TRUE;
#endif

	QApplication app( argc, argv, useGUI );
	QObject::connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );

//	YZDebugBackend::instance()->enableDebugArea("TYZView", false );
	YZDebugBackend::instance()->setDebugOutput( stderr );
	YZDebugBackend::instance()->setDebugLevel( YZ_ERROR_LEVEL );

	QTranslator qt(  0 );
	qt.load(  QString(  "qt_" ) + QTextCodec::locale(), "." );
	app.installTranslator(  &qt );
	QTranslator myapp(  0 );
//	myapp.load(  QString(  "yzis_" ) + QTextCodec::locale(), QString( PREFIX ) + "/share/yzis/locale/" );
	app.installTranslator(  &myapp );

	new TYZSession();
	YZSession::mOptions->setGroup("Global");
	YZOptionValue* o_splash = YZSession::mOptions->getOption("blocksplash");
	bool splash = o_splash->boolean();
	QString initialSendKeys;

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
			TYZSession::me->createBuffer(argv[ i ]);
		} else {
			s = QString( argv[i] );
			if (s == "-h" || s == "--help") {
				printf("Libyzis runner, tester for libyzis %s (see http://www.yzis.org)\n",
					VERSION_CHAR_LONG " " VERSION_CHAR_DATE );
				printf("\nRun me just like you would run kyzis\n");
				printf("%s -c <ex command>\n", argv[0]);
				exit(0);
			} else if (s == "-v" || s == "--version") {
				printf("Libyzis runner, tester for libyzis %s (see http://www.yzis.org)\n",
					VERSION_CHAR_LONG " " VERSION_CHAR_DATE "\n");
				exit(0);
			} else if (s == "-c") {
				QString optArg;
				o_splash->setBoolean( false );
				if (s.length() > 2) optArg = argv[i]+2;
				else if (i < argc-1) optArg = argv[++i];
				initialSendKeys = optArg;
			} else {
				printf("Unrecognised option: %s\n", argv[i] );
				exit(-1);
			}
		}
	}

	if ( !hasatleastone ) {
		YZSession::me->createBuffer();
		YZView* cView = YZSession::me->currentView();
		cView->displayIntro();
	}

	if (initialSendKeys.length()) {
		YZSession::me->sendMultipleKeys( initialSendKeys );
		o_splash->setBoolean( splash );
	} else {
		printf("You must pass at least an yzis command with:\n%s -c <yzis keystroke>\n", argv[0] );
		printf("Example: libyzisrunner -c ':source test_all.lua <ENTER><ESC>:qall!<ENTER>'\n" );
		exit(0);
	}

//	int ret = app.exec();
	printf("[Press enter to finish]\n");
	getchar();
	return 0;
}






