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

#define dbg()    yzDebug("NYzis")
#define err()    yzError("NYzis")

typedef void ( *sighandler_t )( int );

static void sigint(int sig);
static void sigwinch(int sig);
static void cleaning(void);

int
main(int argc, char *argv[])
{
    YZSession::initDebug( argc, argv );

    // ==============[ Create application ]=============
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

	QCoreApplication *app;
    /*
    if ( useGUI )
        app = ( QCoreApplication* )new QApplication( argc, argv );
    else 
    */
	app = new QCoreApplication( argc,argv );
	app->setOrganizationName("Yzis");
	app->setOrganizationDomain("yzis.org");
	app->setApplicationName("NYzis");

    // ==============[ create session ]=============
	NYZSession::createInstance();
    // socket notifier created on the file descriptor 0 (stdin)
    // to catch key events
	QSocketNotifier *socket = new QSocketNotifier(0,QSocketNotifier::Read);
	QObject::connect( socket, SIGNAL( activated( int ) ), static_cast<NYZSession*>(YZSession::self()), SLOT( processInput( int ) ) );

	// Signal handling
	(void) signal(SIGINT, sigint);      /* arrange interrupts to terminate */
	atexit(cleaning);
	(void)signal(SIGWINCH, sigwinch);// ncurses SHOULD handle that

	// we need errorView in order to display error messages if there
	// is a problem loading a file into a buffer
	YZView* errorView = YZSession::self()->createBufferAndView();
    YZSession::self()->parseCommandLine( argc, argv );
	YZSession::self()->deleteView(errorView);

    QTimer::singleShot(0, static_cast<NYZSession*>( YZSession::self() ), SLOT(frontendGuiReady()) );


    // ==============[ let's rock ! ]=============

	return app->exec();
}

static void cleaning(void)
{
	dbg() << "cleaning()" << endl;

	/* ncurses stuff */
	endwin();
	/* other */
	printf("\n"); // prevent prompt to be badly placed after nyzis exits
}


static void sigint(int /*sig*/)
{
	dbg() << "^C caught" << endl;
	// ^c caught -> sends an escape char..
	NYZSession::self()->sendKey( NYZSession::self()->currentView(), "<ESC>","" );
}


static void sigwinch(int /*sig*/)
{
	dbg() << "sigwinch caught" << endl;
	endwin();
	refresh();
	NYZView *view = static_cast<NYZView*>(NYZSession::self()->currentView());
	view->unmap();
	view->map();
	view->refreshScreen();
}


