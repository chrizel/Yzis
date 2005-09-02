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
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
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
#include <qtimer.h>
#include <libintl.h>
#include <locale.h>
#include "libyzis/translator.h"
#include "libyzis/session.h"
#include "libyzis/view.h"
#include "debug.h"
#include "yzis.h"
#include "factory.h"

static const char *description =
I18N_NOOP("KDE Frontend for the Yzis Editor");

static KCmdLineOptions options[] = {
	{ "+[URL]", I18N_NOOP( "Document to open." ), 0 },
	{ "c <argument>", I18N_NOOP( "Execute command passed in argument" ), 0 },
	KCmdLineLastOption
};

int main(int argc, char **argv) {
	KLocale::setMainCatalogue("yzis");
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
	about.addCredit( "Erlend Hamberg", "Misc fixes and improvements", "ehamberg@online.no" );
	about.addCredit( "Per Johansson", "Kyzis Tab caption improvement", "per.j@hjolug.org" );
	about.addCredit( "Helder Correia", "Commands and bug fixes", "helder.correia@netcabo.pt" );
	about.addCredit( "Scott Newton", "Yzisinfo file and tag commands", "scottn@ihug.co.nz" );
	about.addCredit( "Dmitry Suzdalev", "Misc fixes and improvments", "dimsuz@mail.ru" );
	about.addCredit( "Dawid Ciezarkiewicz","Fixed 'xp' command", "<araelx@gmail.com>" );
	about.addCredit( "Craig Howard",":cd and :pwd commands", "<craig@choward.ca>" );
	about.addCredit( "Lars Ivar Igesund","utf8 fixes for Lua strings", "<larsivar@igesund.net>" );

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions( options );
	KApplication app;

	setlocale( LC_ALL, "");
	bindtextdomain( "yzis", QString("%1%2").arg( PREFIX ).arg("/share/locale").toUtf8().data() );
	bind_textdomain_codeset( "yzis", "UTF-8" );
	textdomain( "yzis" );
	// see if we are starting with session management
//	if (app.isRestored())
//				RESTORE(Kyzis)
//	else
	{
		// no session.. just start up normally
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

		QString initialSendKeys = args->getOption("c");

		Kyzis *widget = new Kyzis(0, initialSendKeys);
		kapp->setMainWidget( widget );
		widget->show();
		
		if ( args->count() == 0 ) {
			YZView *view = KYZisFactory::self()->createBufferAndView();
			view->myBuffer()->openNewFile();
			view->displayIntro();
		} else {
			for ( int i = 0; i < args->count(); i++ ) {
				YZView *view = KYZisFactory::self()->createBufferAndView();
				YZSession::me->setCurrentView(view);
				widget->load( args->url( i ) );
			}
		}

		QTimer::singleShot(0, widget, SLOT( init() ));

		args->clear();
	}

	return app.exec();
}
