#ifndef KYZIS_FACTORY_H
#define KYZIS_FACTORY_H

#include <kparts/factory.h>

class KInstance;
class KAboutData;

class KYZisFactory : public KParts::Factory
{
	Q_OBJECT
public:
    KYZisFactory();
    virtual ~KYZisFactory();
	
	KParts::Part *createPartObject (QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList &args );
	
	static const KAboutData *aboutData(); 
    static KInstance* instance();

private:

    static KInstance *s_instance;
    static KAboutData *s_aboutData;
};

#endif 
