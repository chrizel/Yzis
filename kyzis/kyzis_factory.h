#ifndef KYZIS_FACTORY_H
#define KYZIS_FACTORY_H

#include <kparts/factory.h>

class KInstance;
class KAboutData;

class KYZisFactory : public KParts::Factory
{
	Q_OBJECT
public:
    KYZisFactory(bool clone=false);
    virtual ~KYZisFactory();
	
	KParts::Part *createPartObject (QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList &args );
	
		static const KAboutData *aboutData();
		static KInstance* instance();
		static void registerDocument ( class KYZisDoc *doc );
		static void deregisterDocument ( class KYZisDoc *doc );
		static void registerView ( class KYZisView *view );
		static void deregisterView ( class KYZisView *view );

private:
    static void ref();
    static void deref();

    static unsigned long s_refcnt;
		static KYZisFactory *s_self;
    static QPtrList<class KYZisDoc> s_documents;
    static QPtrList<class KYZisView> s_views;
		
    static KInstance *s_instance;
    static KAboutData *s_aboutData;
};

#endif 
