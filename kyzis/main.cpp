/*
    Copyright (c) 2003,2004 Mickael Marchand <marchand@kde.org>

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
#include "translator.h"
#include "debug.h"

static const char *description =
I18N_NOOP("KDE Frontend for the Yzis Editor");

static const char *version = "M1";

static KCmdLineOptions options[] = {
	{ "+[URL]", I18N_NOOP( "Document to open." ), 0 },
	KCmdLineLastOption
};

int main(int argc, char **argv) {
	KAboutData about("kyzis", I18N_NOOP("Kyzis"), version, description, KAboutData::License_GPL, "(C) 2003, 2004 Yzis Team", 0, 0, "yzis-dev@yzis.org");
	about.addAuthor( "Yzis Team", 0, "yzis-dev@yzis.org" );
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
	/*    if (app.isRestored())
				RESTORE(Kyzis)
				else*/
	{
		// no session.. just start up normally
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		QDomDocument domDoc;
		QDomElement dockConfig = domDoc.createElement("dockConfig");
		domDoc.appendChild( dockConfig );

		if ( args->count() == 0 ) {
			Kyzis *widget = new Kyzis(dockConfig,KMdi::TabPageMode);
			widget->createBuffer();
			widget->resize( 600,400 );
			widget->show();
		} else {
			int i = 0;
			Kyzis *widget = new Kyzis(dockConfig,KMdi::TabPageMode);
			widget->resize( 600,400 );
			widget->show();
			for (; i < args->count(); i++ )
				widget->createBuffer( args->url( i ).url() );
		}
		args->clear();
	}

	return app.exec();
}
