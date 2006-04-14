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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
 * $Id: main.cpp 2071 2005-08-31 09:39:49Z mikmak $
 */
#include <qapplication.h>
//#include <ktempfile.h>
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
#include "qyzis.h"
#include "factory.h"


int main(int argc, char **argv) {
	QApplication app(argc, argv);

	QCoreApplication::setOrganizationName("Yzis");
	QCoreApplication::setOrganizationDomain("yzis.org");
	QCoreApplication::setApplicationName("QYzis");

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
		Qyzis *widget = new Qyzis(0);
		widget->show();
		
		/* TODO : handles remaining argc/argv 
		if ( args->count() == 0 ) {
			YZView *view = QYZisFactory::self()->createBufferAndView();
			view->myBuffer()->openNewFile();
			view->displayIntro();
		} else {
			for ( int i = 0; i < args->count(); i++ ) {
				YZView *view = QYZisFactory::self()->createBufferAndView();
				YZSession::me->setCurrentView(view);
				widget->load( args->url( i ) );
			}
		}
		*/
		YZView *view = QYZisFactory::self()->createBufferAndView();
		view->myBuffer()->openNewFile();
		view->displayIntro();

		QTimer::singleShot(0, widget, SLOT( init() ));

		//args->clear();
	}

	return app.exec();
}
