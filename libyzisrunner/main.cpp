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
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <qtextcodec.h>
#include "libyzis/portability.h"
#include "libyzis/session.h"
#include "libyzis/view.h"
#include "libyzis/internal_options.h"

#include "NoGuiSession.h"

int main(int argc, char **argv) {
	bool useGUI = false;

    QCoreApplication *app;
    if ( useGUI ) {
        app = new QApplication( argc, argv );
        QObject::connect( app, SIGNAL(lastWindowClosed()), app, SLOT(quit()) );
    } else {
        app = new QCoreApplication( argc,argv );
    }

    QStringList slArgv;
    for( int i=0; i<argc; i++) {
        slArgv << argv[i];
    }

    /*
    QList<int> ll;
    foreach( int s, ll ) qDebug("s=%d", s );
    */

    setlocale( LC_ALL, "");
    QString l = QString(PREFIX) + "/share/locale";

    YZDebugBackend::self()->parseRcfile( DEBUGRC_FNAME );
    YZDebugBackend::self()->parseArgv( slArgv );
    yzDebug() << " ==============[ libyzisrunner started at: " << QDateTime::currentDateTime().toString() << "]====================" << endl;

	NoGuiSession::createInstance();

    YZSession::self()->getOptions()->setGroup("Global");
    YZOptionValue* o_splash = YZSession::self()->getOptions()->getOption("blocksplash");
    bool splash = o_splash->boolean();
    QString initialSendKeys;

    /*
     * Open buffers
     */
    bool hasatleastone = false;
    QString s;

    /** read options **/
    for ( int i=1; i<slArgv.count(); i++ ) {
	    if ( '-' != slArgv[i][0] ) {
		    hasatleastone = true;
		    yzDebug("NoGuiYzis")<< "NoGuiYzis : opening file " << slArgv[i]<<endl;
		    NoGuiSession::self()->createBuffer(slArgv[ i ]);
	    } else {
		    s = slArgv[i];
		    if (s == "-h" || s == "--help") {
			    printf("Libyzis runner, tester for libyzis %s (see http://www.yzis.org)\n",
					    VERSION_CHAR_LONG " " VERSION_CHAR_DATE );
			    printf("\nRun me just like you would run kyzis\n");
			    printf("%s -c <ex command>\n", qp(slArgv[0]));
			    exit(0);
		    } else if (s == "-v" || s == "--version") {
			    printf("Libyzis runner, tester for libyzis %s (see http://www.yzis.org)\n",
					    VERSION_CHAR_LONG " " VERSION_CHAR_DATE "\n");
			    exit(0);
		    } else if (s == "-c") {
			    QString optArg;
			    o_splash->setBoolean( false );
			    if (s.length() > 2) optArg = slArgv[i].mid(2);
			    else if (i < slArgv.count()-1) optArg = slArgv[++i];
			    initialSendKeys = optArg;
		    } else {
			    printf("Unrecognised option: %s\n", qp(slArgv[i]) );
			    exit(-1);
		    }
	    }
    }

    if ( !hasatleastone ) {
	    YZView *cView = YZSession::self()->createBufferAndView();
	    cView->displayIntro();
    }

    if (initialSendKeys.length()) {
	    YZSession::self()->sendMultipleKeys( initialSendKeys );
	    o_splash->setBoolean( splash );
    } else {
	    printf("You must pass at least an yzis command with:\n%s -c <yzis keystroke>\n", qp(slArgv[0]) );
	    printf("Example: libyzisrunner -c ':source test_all.lua <ENTER><ESC>:qall!<ENTER>'\n" );
	    goto proper_exit;
    }

proper_exit:
    //	int ret = app.exec();
    printf("[Press enter to finish]\n");
    getchar();
    return 0;
}

