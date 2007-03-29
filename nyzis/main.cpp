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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* System */
#include <getopt.h> // getopt
#include <locale.h>
#include <cstdlib>
#include <cstdio>
#include <csignal>

/* Qt */
#include <qglobal.h>
#include <QWidget>
#include <QCoreApplication>
#include <QTextCodec>
#include <QDateTime>
#include <QSocketNotifier>
#include <qtimer.h>

/* libyzis */
#include "libyzis/translator.h"
#include "buffer.h"

/* nyzis */
#include "debug.h"
#include "nsession.h"

/* X11 */
#include <X11/Xlib.h>
#include <libintl.h>


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
	bool useGUI = true;
#endif

    YZDebugBackend::self()->parseRcfile( DEBUGRC_FNAME );
    YZDebugBackend::self()->parseArgv( argc, argv );
    yzDebug() << QDateTime::currentDateTime().toString() << endl;

	QCoreApplication *app;
/*if ( useGUI )
	app = ( QCoreApplication* )new QApplication( argc, argv );
else */
	app = new QCoreApplication( argc,argv );

	QSocketNotifier *socket = new QSocketNotifier(0,QSocketNotifier::Read);

	QString initialSendKeys;

	setlocale( LC_ALL, "");
	QString l = QString(PREFIX) + "/share/locale";
	bindtextdomain( "yzis", l.toUtf8() );
	bind_textdomain_codeset( "yzis", "UTF-8" );
	textdomain( "yzis" );
	// option stuff
	int option_index = 0;
	static struct option long_options[] = { // 0,1,2 = no_argument,required_argument,optional_argument
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};

	int c;
	
	while ( 1 ) {
		c = getopt_long ( argc, argv, "hvc:", long_options, &option_index );
		if ( -1 == c ) break; // end of parsing
		switch (c) {
			case 'h':
				endwin();
				printf("Nyzis, ncurses part of Yzis - http://www.yzis.org\n"
					VERSION_CHAR_LONG " " VERSION_CHAR_DATE );
				printf("\nUsage : %s [--help|-h] [--version|-v] [filename1 [filename2] .... ]\n", argv[0]);
				exit(0);
				break;
			case 'v':
				endwin();
				printf("Nyzis, ncurses part of Yzis - http://www.yzis.org\n"
					VERSION_CHAR_LONG " " VERSION_CHAR_DATE "\n");
				exit(0);
				break;
			case 'c':
				initialSendKeys = (const char *) optarg;
				break;
			default:
				printf ( "?? getopt returned character code 0%o ??\n", c );
		}

	};

	// create session
	NYZSession::createInstance( "default_session", initialSendKeys );

	NYZSession *session = static_cast<NYZSession*>(NYZSession::self());

	QObject::connect( socket, SIGNAL( activated( int ) ),session, SLOT( processInput( int ) ) );

	// Signal handling
	(void) signal(SIGINT, sigint);      /* arrange interrupts to terminate */
	atexit(cleaning);
	(void)signal(SIGWINCH, sigwinch);// ncurses SHOULD handle that

	/*
	 * Open buffers. Create an empty view that will show error messages
	 * in case something goes wrong when opening a file.
	 */
        
	YZView* errorBuffer = session->createBufferAndView();
	YZView* first = NULL;
	YZView* v;

	for ( int i=1; i<argc; i++ ) {
		if ( '-' != argv[i][0] ) {
			yzDebug(NYZIS)<< "nyzis : opening file " << argv[i]<<endl;
			v = session->createBufferAndView(argv[ i ]);
			if ( !first )
				first = v;
		}
	}

	if ( !first ) {
		first = session->createBufferAndView();
		first->myBuffer()->openNewFile();
		first->displayIntro();
	}
		
	session->setCurrentView( first );
	session->deleteView(errorBuffer);

	QTimer::singleShot( 0, session, SLOT( init() ) );

	session->guiStarted();

	return app->exec();
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
	NYZSession::self()->currentView()->sendKey( "<ESC>","" );
}


static void sigwinch(int /*sig*/)
{
//	yzDebug(NYZIS) << "sigwinch catched" << endl;
	endwin();
	refresh();
	NYZView *view = static_cast<NYZView*>(NYZSession::self()->currentView());
	view->unmap();
	view->map();
	view->refreshScreen();
}


