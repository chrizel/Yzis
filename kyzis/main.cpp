/*
    Copyright (c) 2003-2005 Mickael Marchand <marchand@kde.org>

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
#include "kyzis.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include "libyzis/translator.h"
#include "libyzis/session.h"
#include "libyzis/view.h"
#include "debug.h"
#include "yzis.h"

static const char *description =
I18N_NOOP("KDE Frontend for the Yzis Editor");

static KCmdLineOptions options[] = {
	{ "+[URL]", I18N_NOOP( "Document to open." ), 0 },
	{ "c <argument>", I18N_NOOP( "Execute command passed in argument" ), 0 },
	KCmdLineLastOption
};

int main(int argc, char **argv) {
	KAboutData about("kyzis", I18N_NOOP("Kyzis"), VERSION_CHAR, description, KAboutData::License_GPL_V2, 0, 0, "http://www.yzis.org", "bugs@bugs.yzis.org");
	about.addAuthor( "Mickael Marchand", "Author", "mikmak@yzis.org" );
	about.addAuthor( "Thomas Capricelli", "Author", "orzel@freehackers.org" );
	about.addAuthor( "Philippe Fremy", "Author", "phil@freehackers.org" );
	about.addAuthor( "Loic Pauleve", "Author", "panard@inzenet.org" );
	about.addCredit( "Thomas Nagy", "Icons, KSettings", "tnagy@eleve.emn.fr" );
	about.addCredit( "Lucijan Bush", "EX mode history", "lucijan@kde.org" );
	about.addCredit( "Mathieu Kooiman", "KTextEditor interfaces / Dutch translation", "M.Kooiman@map-is.nl" );
	about.addCredit( "Pascal Maillard", "Commands and motion engine rewrite and cleanup", "pascalmaillard@web.de" );
	about.addCredit( "Alexander Dymo", "KTextEditor ConfigInterface", "adymo@mksat.net" );
	about.addCredit( "Erlend Hamberg", "View scrolling fixes", "ehamberg@online.no" );
	about.addCredit( "Per Johansson", "Kyzis Tab caption improvement", "per.j@hjolug.org" );
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions( options );
	KApplication app;


	QTranslator qt(  0 );
	qt.load(  QString(  "qt_" ) + QTextCodec::locale(), "." );
	app.installTranslator(  &qt );
	QTranslator myapp(  0 );
	myapp.load(  QString(  "yzis_" ) + QTextCodec::locale(), QString( PREFIX ) + "/share/yzis/locale/" );
	app.installTranslator(  &myapp );

	// see if we are starting with session management
//	if (app.isRestored())
//				RESTORE(Kyzis)
//	else
	{
		// no session.. just start up normally
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		QDomDocument domDoc;
		QDomElement dockConfig = domDoc.createElement("dockConfig");
		domDoc.appendChild( dockConfig );

		Kyzis *widget = new Kyzis(dockConfig,KMdi::IDEAlMode);
		kapp->setMainWidget( widget );
		widget->show();

		if ( args->count() == 0 ) {
			widget->createBuffer();
		} else {
			int i = 0;
			for (; i < args->count(); i++ )
				widget->createBuffer( args->url( i ).url() );
		}

		QString initialSendKeys = args->getOption("c");
		YZSession::mOptions->setGroup("Global");
		bool splash = YZSession::getBoolOption("blocksplash");
		YZSession::setBoolOption("blocksplash", false);
		if (initialSendKeys.length()) {
			YZSession::me->sendMultipleKeys(initialSendKeys);
		}
		YZSession::setBoolOption("blocksplash", splash);

		args->clear();
	}

	return app.exec();
}
