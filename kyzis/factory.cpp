/*  This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <mikmak@yzis.org>,
 *  Lucijan Busch <lucijan@kde.org>
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

#include "factory.h"
#include "document.h"
#include "viewwidget.h"
#include "session.h"
#include "debug.h"
#include <ktexteditor/document.h>
#include <kmessagebox.h>

YZSession *KYZisFactory::sess = 0;
KYZisView *KYZisFactory::currentView=0;
KYZisDoc *KYZisFactory::currentDoc=0;
KYZisFactory *KYZisFactory::s_self = 0;
unsigned long int KYZisFactory::s_refcnt = 0;
QPtrList<class KYZisDoc> KYZisFactory::s_documents;
QPtrList<class KYZisView> KYZisFactory::s_views;

extern "C" {
	void* init_libkyzispart() {
		KGlobal::locale()->insertCatalogue("kyzispart");
		return new KYZisFactory(true);
	}
};

KInstance* KYZisFactory::s_instance = 0L;

KYZisFactory::KYZisFactory( bool clone ) {
	if ( !sess )
		sess = new YZSession();

	sess->registerManager(this);

	if ( clone ) {
		ref();
		return;
	}
}

KYZisFactory::~KYZisFactory() {
	if ( s_self == this ) {
		delete s_instance->aboutData();
		delete s_instance;
		s_instance=0;
	}	else 
		deref();
}

void KYZisFactory::deref() {
	if ( !--s_refcnt && s_self )
	{
		delete s_self;
		s_self = 0;
	}
}

void KYZisFactory::ref() {
	if ( !s_refcnt && !s_self )
		s_self = new KYZisFactory;

	s_refcnt++;
}

KParts::Part *KYZisFactory::createPartObject( QWidget *parentWidget, const char *widgetname, 
		QObject *parent, const char *name, const char *classname, const QStringList & /*args*/) {
	yzDebug() << "Factory::createPartObject" << endl;
	bool bSingleView = (classname!=QString("KTextEditor::Document"));
	bool bWantBrowserView =  (classname == QString("Browser/View") );
	bool bWantReadOnly = (bWantBrowserView || ( classname == QString("KParts::ReadOnlyPart") ));

	KYZisDoc *doc = new KYZisDoc (bSingleView, bWantBrowserView, bWantReadOnly, parentWidget, widgetname, parent, name );
	//separate
	if ( bSingleView ) {
		yzDebug() << "KYzisDoc constructor creating a default view for this buffer" << endl;
		KTextEditor::View *view = doc->createView( parentWidget, widgetname );
		doc->insertChildClient( view );
		view->show();
		doc->setBaseWidget( view );
	}

	doc->setReadWrite( !bWantReadOnly );
	return doc;
}

void KYZisFactory::registerDocument ( KYZisDoc *doc ) {
  if ( !s_documents.containsRef( doc ) ) {
    s_documents.append( doc );
    ref();
  }
	currentDoc = doc;
}

void KYZisFactory::deregisterDocument ( KYZisDoc *doc ) {
  if ( s_documents.removeRef( doc ) )
    deref();
}

void KYZisFactory::registerView ( KYZisView *view ) {
  if ( !s_views.containsRef( view ) ) {
    s_views.append( view );
    ref();
  }
	currentView = view;
}

void KYZisFactory::deregisterView ( KYZisView *view ) {
  if ( s_views.removeRef( view ) )
    deref();
}

KInstance* KYZisFactory::instance() {
  if( !s_instance )
    s_instance = new KInstance( aboutData() );
  return s_instance;
}

const KAboutData *KYZisFactory::aboutData() {
	KAboutData *data = new KAboutData ("kyzispart", I18N_NOOP("KYZis"), "M1",
					I18N_NOOP( "KYZis - KDE Frontend for YZis" ),
					KAboutData::License_LGPL_V2,
					I18N_NOOP( "(c) 2002,2003,2004" ), 0, "http://www.yzis.org");
	data->addAuthor ("Mickael Marchand", I18N_NOOP("KTextEditor implementation"), "marchand@kde.org");
	data->setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"), I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));

	return data;
}

//GUI itf
void KYZisFactory::setFocusMainWindow() {
	currentView->editor->setFocus();
}

void KYZisFactory::postEvent(yz_event) {
	QCustomEvent *myev = new QCustomEvent (QEvent::User);
	QApplication::postEvent( this, myev ); //this hopefully gives Qt the priority before processing our own events
}

//receives previously generated events from Qt event loop. hopefully it will do
//what I want :)
void KYZisFactory::customEvent (QCustomEvent *) {
	while ( true ) {
		yz_event event = sess->fetchNextEvent();
		YZView *vi = sess->findView( event.view );
		if ( vi == NULL ) {
			//THIS CANNOT HAPPEN, IF THIS HAPPENS YOU BUGGED YZIS !
			yzDebug() << " Factory : View " << event.view << " NOT found , event type : " << event.id << endl;
			return;
		}
		KYZisView *v = static_cast<KYZisView*> ( vi );
		yzDebug() << "Handling event for view " << event.view << endl;
		QString str;
		switch ( event.id ) {
			case YZ_EV_INVALIDATE_LINE:
				yzDebug() << "event INVALIDATE_LINE " << event.id << endl;
				str = v->myBuffer()->data( event.invalidateline.y );
				if ( str.isNull() ) return;//XXX remove me, should not happen
				v->editor->setTextLine(event.invalidateline.y, str);
				break;
			case YZ_EV_SET_CURSOR:
				yzDebug() << "event SET_CURSOR " << event.id << endl;
				v->editor->setCursor (event.setcursor.x, event.setcursor.y);
				v->status->changeItem( QString("%1,%2-%3 (%4)").arg(event.setcursor.x ).arg( event.setcursor.y ).arg( event.setcursor.y2 ).arg( event.setcursor.percentage),99 );
				break;
			case YZ_EV_SET_STATUS:
				yzDebug() << "event SET_STATUS " << event.id << " " << event.setstatus.text <<  endl;
				v->status->changeItem( event.setstatus.text,0);
				break;
			case YZ_EV_REDRAW:
				yzDebug() << "event REDRAW " << event.id << endl;
				v->editor->updateContents();
				break;
			case YZ_EV_NOOP:
				yzDebug() << "event NOOP " << event.id << endl;
				return;
		}
	}
}

void KYZisFactory::setFocusCommandLine() {
	currentView->command->setFocus();
}

void KYZisFactory::scrollDown( int lines ) {
	yzDebug() << "ScrollDown " << lines <<endl;
	currentView->editor->scrollBy(0, lines * currentView->editor->fontMetrics().lineSpacing());
	currentView->editor->update();
}

void KYZisFactory::scrollUp ( int lines ) {
	yzDebug() << "ScrollUp " << lines <<endl;
	currentView->editor->scrollBy(0, -1 * lines * currentView->editor->fontMetrics().lineSpacing());
	currentView->editor->update();
}

void KYZisFactory::setCommandLineText( const QString& text ) {
	currentView->command->setText( text );
}

QString KYZisFactory::getCommandLineText() const {
	return currentView->command->text();
}

void KYZisFactory::quit( bool ) {
	kapp->quit();
}

void KYZisFactory::setCurrentView( YZView* view ) {
	yzDebug() << "Kyzis : setCurrentView " << view->myId << endl;
	currentView = static_cast<KYZisView*>( view );
	sess->currentViewChanged( view );
	currentView->setActiveWindow();
	//DCOP call to KMdi XXX
#if 0
	DCOPClient *client = kapp->dcopClient();
	QByteArray data;
	QDataStream arg(data, IO_WriteOnly);
	arg << path; // view ID ?
	bool w = client->send(client->appId(), "Kyzis", "activateView(param ?)", data);
	if (w) {
		yzDebug() << "DCOP call successful for " << client->appId() << " to activate view on " << path << endl;
		//finds the buffer
	} else {
		yzDebug() << "DCOP call failed for " << client->appId() << endl;
		//popup error
	}
#endif
}

YZView* KYZisFactory::createView( YZBuffer *buffer ) {
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
	DCOPClient *client = kapp->dcopClient();
	QByteArray data;
	QDataStream arg(data, IO_WriteOnly);
	arg << path;
	bool w = client->send(client->appId(), "Kyzis", "createBuffer(QString)", data);
	if (w) {
		yzDebug() << "DCOP call successful for " << client->appId() << " to create buffer on " << path << endl;
	} else {
		yzDebug() << "DCOP call failed for " << client->appId() << endl;
		sess->mGUI->popupMessage( "DCOP communication is broken ! KYzis is not able to create new buffers" );
		return NULL; //we failed
	}
	return sess->findBuffer( path );
}

void KYZisFactory::popupMessage( const QString& message ) {
	KMessageBox::information(currentView, message, "Error");
}

#include "factory.moc"
