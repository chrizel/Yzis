#ifndef KYZIS_FACTORY_H
#define KYZIS_FACTORY_H

#include <kparts/factory.h>
#include "gui.h"

class KInstance;
class KAboutData;

class KYZisFactory : public KParts::Factory, public Gui
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

		//GUI interface
		void postEvent (yz_event);
		void scrollDown( int l=1 );
		void scrollUp( int l=1 );
		void setCommandLineText( const QString& text );
		QString getCommandLineText() const;
		void setFocusCommandLine();
		void setFocusMainWindow();
		void quit(bool save=true);
		void setCurrentView( YZView* );
		YZView *createView ( YZBuffer* );
	
protected:
		void customEvent( QCustomEvent * );

private:
    static void ref();
    static void deref();

    static unsigned long s_refcnt;
		static KYZisFactory *s_self;
		//doh , QPtrList are evil , drop them ! XXX
    static QPtrList<class KYZisDoc> s_documents;
    static QPtrList<class KYZisView> s_views;
		
    static KInstance *s_instance;
    static KAboutData *s_aboutData;

public:
		static YZSession *sess;
		static KYZisView *currentView;
		static KYZisDoc *currentDoc;
};

#endif 
