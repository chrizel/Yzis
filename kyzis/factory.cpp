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
	bool bSingleView = (classname!=QString("KTextEditor::Document"));
	bool bWantBrowserView =  (classname == QString("Browser/View") );
	bool bWantReadOnly = (bWantBrowserView || ( classname == QString("KParts::ReadOnlyPart") ));

	KParts::ReadWritePart *part = new KYZisDoc (bSingleView, bWantBrowserView, bWantReadOnly, parentWidget, widgetname, parent, name );
	//yzDebug() << "Parent Name : " << parentWidget->name() << endl;
	part->setReadWrite( !bWantReadOnly );
	return part;
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
	KAboutData *data = new KAboutData ("kyzispart", I18N_NOOP("KYZis"), "0.0.1",
					I18N_NOOP( "KYZis - KDE Frontend for YZis" ),
					KAboutData::License_LGPL_V2,
					I18N_NOOP( "(c) 2002" ), 0, "http://www.yzis.org");
	data->addAuthor ("Mickael Marchand", I18N_NOOP("KTextEditor implementation"), "marchand@kde.org");
	data->setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"), I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));

	return data;
}

//GUI itf
void KYZisFactory::setFocusMainWindow() {
	currentView->editor->setFocus();
}

void KYZisFactory::postEvent(yz_event /*ev*/) {
	QCustomEvent *myev = new QCustomEvent (QEvent::User);
	QApplication::postEvent( this, myev ); //this hopefully gives Qt the priority before processing our own events
}

//receives previously generated events from Qt event loop. hopefully it will do
//what I want :)
void KYZisFactory::customEvent (QCustomEvent *) {
	while ( true ) {
		yz_event event = sess->fetchNextEvent();
		YZView *vi = sess->findView( event.view );
		KYZisView *v = static_cast<KYZisView*> ( vi );
		QString str;
		switch ( event.id ) {
			case YZ_EV_INVALIDATE_LINE:
				str = v->myBuffer()->findLine( event.invalidateline.y );
				if ( str.isNull() ) return;
				v->editor->setTextLine(event.invalidateline.y, str);
				break;
			case YZ_EV_SET_CURSOR:
				yzDebug() << "event SET_CURSOR" << endl;
				v->editor->setCursor (event.setcursor.x, event.setcursor.y);
				v->status->changeItem( QString("%1,%2-%3 (%4)").arg(event.setcursor.x ).arg( event.setcursor.y ).arg( event.setcursor.y2 ).arg( event.setcursor.percentage),99 );
				break;
			case YZ_EV_SET_STATUS:
				yzDebug() << "event SET_STATUS" << event.setstatus.text <<  endl;
				v->status->changeItem( event.setstatus.text,0);
				break;
			case YZ_EV_REDRAW:
				v->editor->updateContents();
				break;
			case YZ_EV_NOOP:
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
	yzDebug() << "setCurrentView " << endl;
	currentView->hide(); //simple enough ?
	currentView = static_cast<KYZisView*>( view );
	currentView->show();
}

YZView* KYZisFactory::createView( YZBuffer *buffer ) {
  KYZisDoc *doc = static_cast<KYZisDoc*>(buffer);
//	yzDebug() << "Test2 : " << doc->parentWidget()->name() << endl;
	KTextEditor::View* v = doc->createView(currentDoc->parentWidget());
	return dynamic_cast<YZView*>( v );
}

YZBuffer *KYZisFactory::createBuffer(const QString& path) {
	DCOPClient *client = kapp->dcopClient();
	QByteArray data, reply;
	QCString replytype="";
	QDataStream arg(data, IO_WriteOnly);
	client->attach();
	arg << path;
	bool w = client->call(client->appId(), "Kyzis", "createBuffer", data, replytype, reply, true );
	if (w) {
		//finds the buffer
	} else {
		//popup error
	}
	
	//DCOP between main app and part to allow this function to ask the main app to create the buffer (pfff)
/*	KParts::ReadWritePart *m_part = static_cast<KParts::ReadWritePart *>(createPart(currentDoc->parentWidget(),"",this, "", "KParts::ReadWritePart" ));
	KYZisDoc *b = static_cast<KYZisDoc*>( m_part );
	b->setPath( path );
	b->load();
	sess->addBuffer( b );
	return b;*/
	return NULL;
}

#include "factory.moc"
