/*  This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
 *  Copyright (C) 2003-2004 Lucijan Busch <lucijan@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#include <klocale.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <assert.h>
#include <unistd.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <qstring.h>
#include <ktexteditor/document.h>
#include <kmessagebox.h>
#include <kstaticdeleter.h>
#include <qtimer.h>
#include <qtextcodec.h>

#include "settings.h"
#include "factory.h"
#include "document.h"
#include "viewwidget.h"
#include "debug.h"
#include "yzis.h"

KYZisDoc *KYZisFactory::currentDoc=0;

class KYZisPublicFactory : public KParts::Factory {
	public :
		KParts::Part *createPartObject( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList &args ) {
			return KYZisFactory::self()->createPartObject( parentWidget, widgetName, parent, name, classname, args );
		}
};

K_EXPORT_COMPONENT_FACTORY( libkyzispart, KYZisPublicFactory)

KYZisFactory::KYZisFactory() :
		KParts::Factory(Kyzis::me),
		YZSession("kyzis"),
		m_aboutData("kyzispart", I18N_NOOP("Kyzis"), VERSION_CHAR, I18N_NOOP("Embeddable vi-like editor component"),
		KAboutData::License_LGPL_V2, I18N_NOOP("(c)2002-2005 The Kyzis Authors"),0,"http://www.yzis.org"),
		m_instance( &m_aboutData ),
		lastId(YZViewId::invalid)
{
	m_aboutData.addAuthor ("Mickael Marchand", I18N_NOOP("Initial Author"), "marchand@kde.org");
	m_aboutData.addAuthor ("Thomas Capricelli", I18N_NOOP("Initial Author"), "orzel@freehackers.org");
	m_aboutData.addAuthor ("Philippe Fremy", I18N_NOOP("Initial Author"), "phil@freehackers.org");
	m_aboutData.addAuthor ("Loic Pauleve", I18N_NOOP("Initial Author"), "panard@inzenet.org");
	m_aboutData.addCredit( "Thomas Nagy", I18N_NOOP( "Icons, KSettings" ), "tnagy@eleve.emn.fr" );
	m_aboutData.addCredit( "Lucijan Bush", I18N_NOOP( "EX mode history" ), "lucijan@kde.org" );
	m_aboutData.addCredit( "Mathieu Kooiman", I18N_NOOP( "KTextEditor interfaces / Dutch translation" ), "M.Kooiman@map-is.nl" );
	m_aboutData.addCredit( "Pascal Maillard", I18N_NOOP( "Commands and motion engine rewrite and cleanup" ), "pascalmaillard@web.de" );
	m_aboutData.addCredit( "Alexander Dymo", I18N_NOOP( "KTextEditor ConfigInterface" ), "adymo@mksat.net" );
	m_aboutData.addCredit( "Erlend Hamberg", I18N_NOOP( "View scrolling fixes" ), "ehamberg@online.no" );
	m_aboutData.addCredit( "Per Johansson", I18N_NOOP( "Kyzis Tab caption improvement" ), "per.j@hjolug.org" );
	m_aboutData.addCredit( "Helder Correia", I18N_NOOP( "Commands and bug fixes" ), "helder.correia@netcabo.pt" );
	m_aboutData.addCredit( "Scott Newton", I18N_NOOP( "Yzisinfo file and tag commands" ), "scottn@ihug.co.nz" );
	m_aboutData.addCredit( "Dmitry Suzdalev", I18N_NOOP( "Misc fixes and improvements" ), "dimsuz@mail.ru" );
	m_aboutData.addCredit( "Dawid Ciezarkiewicz",I18N_NOOP( "Fixed 'xp' command" ), "araelx@gmail.com" );
	m_aboutData.addCredit( "Craig Howard",I18N_NOOP( ":cd and :pwd commands" ), "craig@choward.ca" );
	m_aboutData.addCredit( "Lars Ivar Igesund",I18N_NOOP( "utf8 fixes for Lua strings" ), "larsivar@igesund.net" );
	m_aboutData.addCredit( "Christoph Cullmann",I18N_NOOP( "help to port to KDE4" ), "cullmann@kde.org" );
	m_aboutData.addCredit( "Dawid Ciezarkiewicz",I18N_NOOP( "D language syntax highlight fixes" ), "araelx@gmail.com" );
	m_aboutData.setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"), I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));

	Settings::self()->readConfig();
	guiStarted();
}

KYZisFactory::~KYZisFactory() {
	kdDebug() << "Factory gets destroyed !" << endl;
}

static KStaticDeleter<KYZisFactory> sdFactory;

KYZisFactory *KYZisFactory::self() {
	static KYZisFactory *self = 0;

	if ( !self ) {
		self = new KYZisFactory;
	}
		
	return self;
}

KParts::Part *KYZisFactory::createPartObject( QWidget *parentWidget, const char *widgetname, QObject *parent, const char *name, const char *_classname, const QStringList &/*args*/) {
	yzDebug() << "Factory::createPartObject" << endl;
	QCString classname (_classname);
	bool bSingleView = (classname!="KTextEditor::Document");
	yzDebug() << "Factory singleView  " << bSingleView << endl;

	if ( parentWidget && parentWidget->inherits( "Kyzis" ) )
		Kyzis::me = static_cast<Kyzis*>( parentWidget );
	else {
        Kyzis::me = 0;
	}

	KYZisDoc *doc = new KYZisDoc (parentWidget, widgetname, parent, name );
	doc->setState( YZBuffer::ACTIVE );
	//separate
	if ( bSingleView ) {
		yzDebug() << "Factory creates single view ..." << endl;
		KTextEditor::View *view = doc->createView( parentWidget, widgetname );
		KYZisView *yv = static_cast<KYZisView*>( view );
		doc->insertChildClient( view );
		view->show();
		doc->setBaseWidget( view );
		
		addView( yv );
	}
	doc->filenameChanged();

	doc->setReadWrite( true );
	
	addBuffer( doc );
	
	return doc;
}

const KAboutData *KYZisFactory::aboutData() {
	return &m_aboutData;
}

bool KYZisFactory::quit( int /*errorCode*/ ) {
	//a kpart CAN NOT exit the main app ;)
	if (Kyzis::me) {
		kapp->quit();
	} else if (kapp->name() == QString::fromLatin1("kdevelop") ) {
		/*
		for (int i = 0; i < YZSession::mNbViews; i++)
			deleteView(i);
		*/
	} else if ( currentView() && currentView()->modePool()->currentType() == YZMode::MODE_EX 
				&& !currentView()->getCommandLineText().isEmpty() ) {
		return false;
	}		


	return true;
}

void KYZisFactory::writeConfig() {
	Settings::self()->writeConfig();
	applyConfig();
}

void KYZisFactory::applyConfig() {
	// apply new configuration to all views
	YZBufferList::ConstIterator it = buffers().begin();
	YZBufferList::ConstIterator end = buffers().end();
	for ( ; it != end; ++it ) {
		YZBuffer *b = *it;
		YZList<YZView*> l = b->views();
		for ( YZList<YZView*>::Iterator itr = l.begin(); itr != l.end(); ++itr ) {
			KYZisView* yv = static_cast<KYZisView*>( *itr );
			yv->applyConfig();
		}
	}
}

void KYZisFactory::readConfig( ) {
	Settings::self()->readConfig();
	applyConfig();
}

void KYZisFactory::changeCurrentView( YZView* view ) {
	yzDebug() << "Kyzis : setCurrentView " << view->getId() << endl;
	KYZisView *v = static_cast<KYZisView*>(view);
	v->setActiveWindow();
	v->setFocus();
}

YZView* KYZisFactory::doCreateView( YZBuffer *) {
	//DCOP call which returns the UID of the created view ?
#if 0
	DCOPClient *client = kapp->dcopClient();
	QByteArray data;
	QDataStream arg(data, IO_WriteOnly);
	arg << buffer->fileName();
	bool w = client->send(client->appId(), "Kyzis", "createView(QString)", data);
	if (w) {
		yzDebug() << "DCOP call successful for " << client->appId() << " to create view on " << buffer->fileName() << endl;
	} else {
		yzDebug() << "DCOP call failed for " << client->appId() << endl;
		sess->mGUI->popupMessage( "DCOP communication is broken ! KYzis is not able to create new views" );
		return NULL;
	}
#endif
	return NULL;
}

YZBuffer *KYZisFactory::createBuffer(const QString& path /*=QString::null*/) {
#if 0
	DCOPClient *client = kapp->dcopClient();
	QByteArray data;
	QDataStream arg(data, IO_WriteOnly);
	arg << path;
	bool w = client->send(client->appId(), "Kyzis", "createBuffer(QString)", data);
	if (w) {
		yzDebug() << "DCOP call successful for " << client->appId() << " to create buffer on " << path << endl;
	} else {
		yzDebug() << "DCOP call failed for " << client->appId() << endl;
		popupMessage( "DCOP communication is broken ! KYzis is not able to create new buffers" );
		return NULL; //we failed
	}
	return findBuffer( path );
#endif
	QString createdPath;

	if (Kyzis::me)
		createdPath = Kyzis::me->createBuffer(path);
	return findBuffer( createdPath );
}

void KYZisFactory::popupMessage( const QString& message ) {
	KYZisView *v = static_cast<KYZisView*>(currentView());
	KMessageBox::information(v, message, _( "Error" ));
}

void KYZisFactory::closeView() {
	if (Kyzis::me) {
		Kyzis::me->closeView(lastId);
		
		removeView( findView( lastId ) );
	}
	else if (kapp->name() == QString::fromLatin1("kdevelop") ) {
		yzDebug() << "Calling kdevelop" <<endl;
		DCOPClient *client = kapp->dcopClient();
		QByteArray data;
		QDataStream arg(data, IO_WriteOnly);
		arg << QCString("file_close");
		bool w = client->send(client->appId(), "MainWindow", "activateAction(QCString)", data);
		if (w) {
			yzDebug() << "DCOP call successful for " << client->appId() << " to close view in kdevelop" << endl;
		} else {
			yzDebug() << "DCOP call failed for " << client->appId() << endl;
			popupMessage( "DCOP communication is broken !" );
		}
			
	}
	lastId = YZViewId::invalid;
}

void KYZisFactory::doDeleteView( YZView *view ) {
	lastId = view->getId();
	QTimer::singleShot(10, this, SLOT( closeView() ));
#if 0
	DCOPClient *client = kapp->dcopClient();
	QByteArray data;
	QDataStream arg(data, IO_WriteOnly);
	arg << Id;
	bool w = client->send(client->appId(), "Kyzis", "closeView(int)", data);
	if (w) {
		yzDebug() << "DCOP call successful for " << client->appId() << " to delete view " << endl;
	} else {
		yzDebug() << "DCOP call failed for " << client->appId() << endl;
		popupMessage( "DCOP communication is broken ! KYzis is not able to delete the current view" );
		return; //we failed
	}
#endif
}

void KYZisFactory::deleteBuffer(YZBuffer* /*b*/) {
}

void KYZisFactory::setFocusMainWindow() {
	KYZisView *yv = static_cast<KYZisView*>( currentView() );
	yv->setFocusMainWindow();
}

void KYZisFactory::setFocusCommandLine() {
	KYZisView *yv = static_cast<KYZisView*>( currentView() );
	yv->setFocusCommandLine();
}

bool KYZisFactory::promptYesNo(const QString& title, const QString& message) {
	int v = KMessageBox::questionYesNo(static_cast<KYZisView*>(currentView()), message, title);
	if (v == KMessageBox::Yes) return true;
	else return false;
}

int KYZisFactory::promptYesNoCancel(const QString& title, const QString& message) {
	int v = KMessageBox::questionYesNoCancel(static_cast<KYZisView*>(currentView()), message, title);
	if (v == KMessageBox::Yes) return 0;
	else if (v == KMessageBox::No) return 1;
	return 2;
}

void KYZisFactory::splitHorizontally(YZView* /*view*/) {
//	KYZisView *v = static_cast<KYZisView *>(view);
//	KYZisDoc *doc = static_cast<KYZisDoc*>(v->document());
	if (Kyzis::me)
		Kyzis::me->createView();
}

#include "factory.moc"
