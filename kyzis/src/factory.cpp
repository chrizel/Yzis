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
Kyzis *KYZisFactory::mMainApp = 0;
QPtrList<class KYZisDoc> KYZisFactory::s_documents;
//QPtrList<class KYZisView> KYZisFactory::s_views;


class KYZisPublicFactory : public KParts::Factory {
	public :
		KParts::Part *createPartObject( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList &args ) {
			return KYZisFactory::self()->createPartObject( parentWidget, widgetName, parent, name, classname, args );
		}
};

K_EXPORT_COMPONENT_FACTORY( libkyzispart, KYZisPublicFactory)

KYZisFactory *KYZisFactory::s_self = 0;

KYZisFactory::KYZisFactory() :
	m_aboutData("kyzispart", I18N_NOOP("Kyzis Part"), VERSION_CHAR, I18N_NOOP("Embeddable vi-like editor component"),KAboutData::License_LGPL_V2, I18N_NOOP("(c)2002-2005 The Kyzis Authors"),0,"http://www.yzis.org"),
	m_instance( &m_aboutData ) {
	s_self = this;
	lastId = -1;
	Settings::self()->readConfig();
	guiStarted();
}

KYZisFactory::~KYZisFactory() {
	kdDebug() << "Factory gets destroyed !" << endl;
	while ( KYZisDoc *doc=s_documents.first() ) {
		kdDebug() << "Deleting " << doc->fileName() << endl;
	    s_self=this;
	    delete doc;
	    s_self=0;
	}
}

static KStaticDeleter<KYZisFactory> sdFactory;

KYZisFactory *KYZisFactory::self() {
	if ( !s_self )
		sdFactory.setObject(s_self, new KYZisFactory());

	return s_self;
}

KParts::Part *KYZisFactory::createPartObject( QWidget *parentWidget, const char *widgetname, QObject *parent, const char *name, const char *classname, const QStringList &args) {
	yzDebug() << "Factory::createPartObject" << endl;
	bool bSingleView = (classname!=QString("KTextEditor::Document"));
	yzDebug() << "Factory singleView  " << bSingleView << endl;
	//mMainApp = dynamic_cast<Kyzis*>(parentWidget);
	if ( parentWidget && parentWidget->inherits( "Kyzis" ) )
		mMainApp = static_cast<Kyzis*>( parentWidget );
	else
		mMainApp = 0;

	QString kID, kvId;
	kID = args[ 0 ];// buffer ID
	kvId = args[ 1 ]; // view ID

	KYZisDoc *doc = new KYZisDoc (kID.toInt(), parentWidget, widgetname, parent, name );
	//separate
	if ( bSingleView ) {
		yzDebug() << "Factory creates single view ..." << endl;
		KTextEditor::View *view = doc->createView( parentWidget, widgetname );
		KYZisView *yv = static_cast<KYZisView*>( view );
		yv->setkid( kvId.toInt() );
		doc->insertChildClient( view );
		view->show();
		doc->setBaseWidget( view );
	}
	doc->filenameChanged();

	doc->setReadWrite( true );
	return doc;
}

void KYZisFactory::registerDoc( KYZisDoc *doc ) {
	kdDebug() << "Register " << doc->fileName() << endl;
	if ( !s_documents.contains( doc ) )
		s_documents.append( doc );
}

void KYZisFactory::unregisterDoc( KYZisDoc *doc ) {
	kdDebug() << "Unregister " << doc->fileName() << endl;
	if ( s_documents.contains( doc ) )
		s_documents.removeRef( doc );
}

const KAboutData *KYZisFactory::aboutData() {
	KAboutData *data = new KAboutData ("kyzispart", I18N_NOOP("Kyzis"), VERSION_CHAR,
					I18N_NOOP( "Kyzis - KDE Frontend for Yzis" ),
					KAboutData::License_GPL_V2,
					I18N_NOOP( "(c) 2003-2005" ), 0, "http://www.yzis.org");
	data->addAuthor ("Mickael Marchand", I18N_NOOP("Initial Author"), "marchand@kde.org");
	data->addAuthor ("Thomas Capricelli", I18N_NOOP("Initial Author"), "orzel@freehackers.org");
	data->addAuthor ("Philippe Fremy", I18N_NOOP("Initial Author"), "phil@freehackers.org");
	data->addAuthor ("Loic Pauleve", I18N_NOOP("Initial Author"), "panard@inzenet.org");
	data->addCredit( "Thomas Nagy", "Icons, KSettings", "tnagy@eleve.emn.fr" );
	data->addCredit( "Lucijan Bush", "EX mode history", "lucijan@kde.org" );
	data->addCredit( "Mathieu Kooiman", "KTextEditor interfaces / Dutch translation", "M.Kooiman@map-is.nl" );
	data->addCredit( "Pascal Maillard", "Commands and motion engine rewrite and cleanup", "pascalmaillard@web.de" );
	data->addCredit( "Alexander Dymo", "KTextEditor ConfigInterface", "adymo@mksat.net" );
	data->addCredit( "Erlend Hamberg", "View scrolling fixes", "ehamberg@online.no" );
	data->addCredit( "Per Johansson", "Kyzis Tab caption improvment", "per.j@hjolug.org" );
	data->setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"), I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));

	return data;
}

void KYZisFactory::quit( int /*errorCode*/ ) {
	//a kpart CAN NOT exit the main app ;)
	if (mMainApp) {
		kapp->quit();
	}/* else if ( currentView() && currentView()->modePool()->currentType() == YZMode::MODE_EX 
				&& !currentView()->getCommandLineText().isEmpty() ) {
		currentView()->setCommandLineText("");
		currentView()->modePool()->pop();
	}*/
}

void KYZisFactory::writeConfig() {
	Settings::self()->writeConfig();
	applyConfig();
}

void KYZisFactory::applyConfig() {
	// apply new configuration to all views
	QMap<QString,YZBuffer*>::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
		YZBuffer *b = ( it.data() );
		QPtrList< YZView > l = b->views();
		YZView* yit;
		for ( yit = l.first(); yit; yit = l.next() ) {
			KYZisView* yv = static_cast<KYZisView*>( yit );
			yv->applyConfig();
		}
	}
}

void KYZisFactory::readConfig( ) {
	Settings::self()->readConfig();
	applyConfig();
}

void KYZisFactory::changeCurrentView( YZView* view ) {
	yzDebug() << "Kyzis : setCurrentView " << view->myId << endl;
	KYZisView *v = static_cast<KYZisView*>(view);
	v->setActiveWindow();
	v->setFocus();
}

YZView* KYZisFactory::createView( YZBuffer *) {
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

YZBuffer *KYZisFactory::createBuffer(const QString& path) {
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
	if (mMainApp)
		mMainApp->createBuffer(path);
	return findBuffer( path );
}

void KYZisFactory::popupMessage( const QString& message ) {
	KYZisView *v = static_cast<KYZisView*>(currentView());
	KMessageBox::information(v, message, _( "Error" ));
}

void KYZisFactory::closeView() {
	if (mMainApp)
		mMainApp->closeView(lastId);
	lastId = -1;
}

void KYZisFactory::deleteView( int Id ) {
	lastId = Id;
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
#if 0
	yzDebug() << "Factory : Close view id : " << Id << endl;
	if (mMainApp)
		mMainApp->closeView(Id);
#endif	
#if 0 //that does not work since KMdi does not know the view is destroyed
	yzDebug() << "Factory : deleteView " << v->myId << endl;
	KYZisView *vv = static_cast<KYZisView*>( v );
	delete vv;
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
	if (mMainApp)
		mMainApp->createView();
}

#include "factory.moc"
