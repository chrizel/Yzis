/*
	Copyright (c) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
	Copyright (c) 2004-2005 Mickael Marchand <marchand@kde.org>

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
 * $Id$
 */


/* std c */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h> // getopt

/* std c++ */
#include <cstdlib>
#include <cstdio>
#include <csignal>

/* libyzis */
#include "libyzis/translator.h"

/* yzis */
#include "debug.h"
#include "eventloop.h"
#include "factory.h"

/* Qt */
#include <qglobal.h>
#if QT_VERSION < 0x040000
#include <qapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#else
#include <QApplication>
#include <QTranslator>
#include <QTextCodec>
#endif

/* X11 */
#include <X11/Xlib.h>



typedef void ( *sighandler_t )( int );

static void sigint(int sig);
static void sigwinch(int sig);
static void cleaning(void);

int
main(int argc, char *argv[])
{

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

#if QT_VERSION < 0x040000
	// keep this one before QApplication creation, and doesn't declare it as "NYZEventLoop loop();", use new
	( void ) new NYZEventLoop();
#else
	//NYZEventLoop loop;
	( void ) new NYZEventLoop();
#endif

	QApplication app( argc, argv, useGUI );

	// Translator stuff
	QTranslator qt(  0 );
#if QT_VERSION < 0x040000
	qt.load(  QString(  "qt_" ) + QTextCodec::locale(), "." );
#else
	qt.load(  QString(  "qt_" ) + QTextCodec::codecForLocale()->name(), "." );
#endif
	app.installTranslator(  &qt );
	QTranslator translator(  0 );
	QString initialSendKeys;
#if QT_VERSION < 0x040000
	translator.load(  QString(  "yzis_" ) + QTextCodec::locale(), QString( PREFIX ) + "/share/yzis/locale/" );
	yzDebug(NYZIS) << "Locale " << QTextCodec::locale() << endl;
#else
	translator.load(  QString(  "yzis_" ) + QTextCodec::codecForLocale()->name(), QString( PREFIX ) + "/share/yzis/locale/" );
	yzDebug(NYZIS) << "Locale " << QString(QTextCodec::codecForLocale()->name()) << endl;
#endif
	app.installTranslator(  &translator );

	// option stuff
	int option_index = 0;
	static struct option long_options[] = { // 0,1,2 = no_argument,required_argument,optional_argument
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};

	int c;
	YZSession::mOptions.setGroup("Global");
	bool splash = YZSession::getBoolOption("blocksplash");
	while ( 1 ) {
		c = getopt_long ( argc, argv, "hvc:", long_options, &option_index );
		if ( -1 == c ) break; // end of parsing
		switch (c) {
			case 'h':
				printf("Nyzis, ncurses part of Yzis - http://www.yzis.org\n"
					VERSION_CHAR_LONG " " VERSION_CHAR_DATE );
				printf("\nUsage : %s [--help|-h] [--version|-v] [filename1 [filename2] .... ]\n", argv[0]);
				exit(0);
				break;
			case 'v':
				printf("Nyzis, ncurses part of Yzis - http://www.yzis.org\n"
					VERSION_CHAR_LONG " " VERSION_CHAR_DATE "\n");
				exit(0);
				break;
			case 'c':
				YZSession::setBoolOption("blocksplash", false);
				initialSendKeys = (const char *) optarg;
				break;
			default:
				printf ( "?? getopt returned character code 0%o ??\n", c );
		}

	};


	// create factory
	NYZFactory *factory  =new NYZFactory();

	// Signal handling
	(void) signal(SIGINT, sigint);      /* arrange interrupts to terminate */
	atexit(cleaning);
	(void)signal(SIGWINCH, sigwinch);// ncurses SHOULD handle that

	/*
	 * Open buffers
	 */
	NYZisDoc *bf;
	bool hasatleastone = false;

	for ( int i=1; i<argc; i++ ) {
		if ( '-' != argv[i][0] ) {
			hasatleastone = true;
			yzDebug(NYZIS)<< "nyzis : opening file " << argv[i]<<endl;
			bf = factory->createBuffer(argv[ i ]);
		}
	}

	if ( !hasatleastone ) {
		factory->createBuffer();
		YZView* cView = YZSession::me->currentView();
		cView->displayIntro();
	}

	if (initialSendKeys.length()) {
		YZSession::me->sendMultipleKeys( initialSendKeys );
		YZSession::setBoolOption("blocksplash", splash);
	}

	YZSession::me->guiStarted();

	return app.exec();
}

static void cleaning(void)
{
	yzDebug(NYZIS) << "end of nyzis, cleaning" << endl;

	/* ncurses stuff */
	endwin();
	/* other */
	printf("\n"); // prevent prompt to be badly placed after nyzis exits
}


static void sigint(int /*sig*/)
{
//	yzDebug(NYZIS) << "^C catched" << endl;
	// ^c catched -> sends an escape char..
	NYZFactory::currentView->sendKey( "<ESC>","" );
}


static void sigwinch(int /*sig*/)
{
//	yzDebug(NYZIS) << "sigwinch catched" << endl;
	endwin();
	refresh();
	NYZFactory::currentView->unmap();
	NYZFactory::currentView->map();
	NYZFactory::currentView->refreshScreen();
}


