/*
	  Copyright (c) 2003 Yzis Team <yzis-dev@yzis.org>

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


/* Qt */
#include <qapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>


/* yzis */
#include "debug.h"
#include "factory.h"
#include "translator.h"
/* std c */
//#include <unistd.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h> // getopt

/* std c++ */
#include <cstdlib>
#include <cstdio>
#include <csignal>

static void catchsigint(int sig);
static void cleaning(void);
void nyz_init_screen(void);

int
main(int argc, char *argv[])
{

	(void) signal(SIGINT, catchsigint);      /* arrange interrupts to terminate */
	atexit(cleaning);
#ifdef Q_WS_X11
	bool useGUI = getenv(  "DISPLAY" ) != 0;
#else
	bool useGUI = TRUE;
#endif
	QApplication app( argc, argv, useGUI );

	// Translator stuff
	QTranslator qt(  0 );
	qt.load(  QString(  "qt_" ) + QTextCodec::locale(), "." );
	app.installTranslator(  &qt );
	QTranslator myapp(  0 );
	myapp.load(  QString(  "yzis_" ) + QTextCodec::locale(), QString( PREFIX ) + "/share/yzis/locale/" );
	yzDebug(NYZIS) << "Locale " << QTextCodec::locale() << endl;
	app.installTranslator(  &myapp );

	// option stuff
	int option_index = 0;
	static struct option long_options[] = { // 0,1,2 = no_argument,required_argument,optional_argument
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};

	int c;
	while ( 1 ) {
		c = getopt_long ( argc, argv, "hv", long_options, &option_index );
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
			default:
				printf ( "?? getopt returned character code 0%o ??\n", c );
		}

	};


	// create factory
	NYZFactory *factory  =new NYZFactory();

	/*
	 * Open buffers
	 */
	NYZisDoc *bf;
	bool hasatleastone = false;

	while ( --argc )
		if ( '-' != argv[argc][0] ) {
			hasatleastone = true;
			yzDebug(NYZIS)<< "nyzis : opening file " << argv[argc]<<endl;;
			bf = factory->createBuffer();
			bf->load( argv[ argc ] );
		}
	if ( !hasatleastone )
		factory->createBuffer();

	
	// let's go and loop
	factory->event_loop();
	yzFatal(NYZIS) << "Should never reach this point" << endl;
}

static void cleaning(void)
{
	yzDebug(NYZIS) << "end of nyzis, cleaning" << endl;

	/* ncurses stuff */
	endwin();
	/* other */
	printf("\n"); // prevent prompt to be badly placed after nyzis exits
}

static void catchsigint(int /*sig*/)
{
//	yzDebug(NYZIS) << "^C catched" << endl;
	// ^c catched -> sends an escape char.. 
	NYZFactory::currentView->sendKey( Qt::Key_Escape, 0); }

 
