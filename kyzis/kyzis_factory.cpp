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

#include "kyzis_factory.h"
#include "kyzisdoc.h"
#include "kyzisview.h"

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
//	bool bSingleView = (classname!=QString("KTextEditor::Document"));
	bool bWantBrowserView =  (classname == QString("Browser/View") );
	bool bWantReadOnly = (bWantBrowserView || ( classname == QString("KParts::ReadOnlyPart") ));

	KParts::ReadWritePart *part = new KYZisDoc (parentWidget, widgetname, parent, name /*,args*/ );
	part->setReadWrite( !bWantReadOnly );
	return part;
}

void KYZisFactory::registerDocument ( KYZisDoc *doc ) {
  if ( !s_documents.containsRef( doc ) ) {
    s_documents.append( doc );
    ref();
  }
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

#include "kyzis_factory.moc"
