/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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
	YZBuffer *createBuffer(const QString& path);
	void popupMessage( const QString& message );
	void deleteView ();
	
protected:
	void customEvent( QCustomEvent * );

private:
    static void ref();
    static void deref();

    static unsigned long s_refcnt;
	//doh , QPtrList are evil , drop them ! XXX
    static QPtrList<class KYZisDoc> s_documents;
    static QPtrList<class KYZisView> s_views;
		
    static KInstance *s_instance;
    static KAboutData *s_aboutData;

public:
	static KYZisFactory *s_self;
	static YZSession *sess;
	static KYZisDoc *currentDoc;
};

#endif 
