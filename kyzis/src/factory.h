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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/


#ifndef KYZIS_FACTORY_H
#define KYZIS_FACTORY_H

#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/factory.h>
#include <ktexteditor/configpage.h>
#include <kaboutdata.h>
#include <kinstance.h>
#include <kparts/factory.h>
#include <QMap>

#include "session.h"
#include "viewid.h"

class KYZTextEditorIface;
class YZBuffer;
class KYZisView;
class YZViewId;

class KYZisPublicFactory : public KTextEditor::Factory, 
	public KTextEditor::Editor {
	Q_OBJECT
	
	public :
		KYZisPublicFactory( QObject* parent = 0, const char * = 0 );
		virtual ~KYZisPublicFactory();

		KParts::Part *createPartObject( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList &args ) ;
		KTextEditor::Editor *editor() ;
		static KYZisPublicFactory* self() ;
		KTextEditor::Document *createDocument(QObject*parent);
		const QList<KTextEditor::Document*> &documents ();
		const KAboutData* aboutData() const;
		void writeConfig();
		void readConfig();
		void writeConfig(KConfig *);
		void readConfig(KConfig *);
		void configDialog ( QWidget * );
		bool configDialogSupported() const;
		int configPages () const;
		KTextEditor::ConfigPage *configPage ( int number, QWidget *parent );
		QString configPageName ( int number ) const;
		QString configPageFullName ( int number ) const;
		QPixmap configPagePixmap ( int number, int size = K3Icon::SizeSmall ) const;
};

class KYZisFactory : public YZSession
{
	Q_OBJECT
public:
	KYZisFactory();
	virtual ~KYZisFactory();

	KParts::Part *createPartObject (QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList &args );

	static KYZisFactory* self();

	inline KInstance* instance() { return &m_instance; }

	//GUI interface
	bool quit(int errorCode);
	void popupMessage( const QString& message );
	void deleteBuffer ( YZBuffer *b );
	void setFocusCommandLine();
	void setFocusMainWindow();
	void splitHorizontally(YZView *view);
	bool promptYesNo(const QString& title, const QString& message);
	int promptYesNoCancel(const QString& title, const QString& message);
	virtual void setClipboardText( const QString& text, Clipboard::Mode mode );
	
	KYZTextEditorIface *createTextEditorIface();
	
	/**
	 * Sets the parent widget for all created KYZisView
	 * This is an ugly hack to get around the deep calls into
	 * YZSession in order to create Views.  These calls cannot
	 * have parent information passed around.  createPartObject
	 * can look in this field to get a parent
	 */
	void setViewParentWidget( QWidget *viewParent ) { m_viewParent = viewParent; }
		//Editor Interface
	const QList<KTextEditor::Document*> &documents(); // { return QList<KTextEditor::Document*>(); } //FIXME
	const KAboutData* aboutData() const { return &m_aboutData; }
	KTextEditor::Document *createDocument(QObject*parent);
	YZView *doCreateView( YZBuffer* buffer );
	void doDeleteView( YZView *view );
	
	/**
	 * Opens a new buffer
	 * @return the newly created buffer
	 */
	YZBuffer* doCreateBuffer();


	void writeConfig();
	void readConfig(); 
	void writeConfig(KConfig *) {}
	void readConfig(KConfig *) {}
	void configDialog ( QWidget * ) { } //for now FIXME TODO FIXME TODO
	bool configDialogSupported() const { return false;} 
	int configPages () const { return 0; }
	KTextEditor::ConfigPage *configPage ( int /*number*/, QWidget */*parent*/ ) { return NULL; }
	QString configPageName ( int /*number*/ ) const { return ""; } 
	QString configPageFullName ( int /*number*/ ) const { return ""; }
	QPixmap configPagePixmap ( int /*number*/, int /*size*/ = K3Icon::SizeSmall ) const { return QPixmap(); }
	
	static KYZTextEditorIface *currentDoc;
	KYZisView *lastView;
	QWidget *m_viewParent;

public slots:
	void applyConfig();
	void closeView();

private:
	KAboutData m_aboutData;
	KInstance m_instance;
	
	void changeCurrentView( YZView* );

	QList<KTextEditor::Document*> m_document;
};

#endif
