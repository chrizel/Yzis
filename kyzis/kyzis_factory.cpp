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

extern "C" {
    void* init_libkyzispart() {
			KGlobal::locale()->insertCatalogue("kyzispart");
			return new KYZisFactory();
    }
};

KInstance* KYZisFactory::s_instance = 0L;

KYZisFactory::KYZisFactory() {
		s_instance=0;
}

KYZisFactory::~KYZisFactory() {
	if ( s_instance ) {
		delete s_instance->aboutData();
		delete s_instance;
	}
	s_instance=0;
}

KParts::Part *KYZisFactory::createPartObject( QWidget *parentWidget, const char *widgetname, 
		QObject *parent, const char *name, const char *classname, const QStringList & args) {
	bool bSingleView = (classname!=QString("KTextEditor::Document"));
	bool bWantBrowserView =  (classname == QString("Browser/View") );
	bool bWantReadOnly = (bWantBrowserView || ( classname == QString("KParts::ReadOnlyPart") ));

	KParts::ReadWritePart *part = new KYZisDoc (parentWidget, widgetname, parent, name /*,args*/ );
	part->setReadWrite( !bWantReadOnly );
	return part;
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
