/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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
#include <kaboutdata.h>
#include <kinstance.h>
#include "session.h"
#include "kyzis.h"
#include "document.h"

class KYZisFactory : public KParts::Factory, public YZSession
{
	Q_OBJECT
public:
	KYZisFactory();
	virtual ~KYZisFactory();

	KParts::Part *createPartObject (QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList &args );

	const KAboutData *aboutData();
	static KYZisFactory* self();

	inline KInstance* instance() { return &m_instance; }

	//GUI interface
	bool quit(int errorCode);
	void changeCurrentView( YZView* );
	YZView *createView ( YZBuffer* );
	YZBuffer *createBuffer(const QString& path);
	void popupMessage( const QString& message );
	void deleteView (int Id);
	void deleteBuffer ( YZBuffer *b );
	void setFocusCommandLine();
	void setFocusMainWindow();
	void splitHorizontally(YZView *view);
	bool promptYesNo(const QString& title, const QString& message);
	int promptYesNoCancel(const QString& title, const QString& message);
	void registerDoc( KYZisDoc *doc );
	void unregisterDoc( KYZisDoc *doc );

public slots :
	void writeConfig();
	void readConfig();
	void applyConfig();
	void closeView();

private:
	//doh , QPtrList are evil , drop them ! XXX
    static QPtrList<class KYZisDoc> s_documents;
    static QPtrList<class KYZisView> s_views;

    KAboutData m_aboutData;
	KInstance m_instance;

public:
	static KYZisFactory *s_self;
	static KYZisDoc *currentDoc;
	static Kyzis *mMainApp;
	int lastId;
};

#endif
