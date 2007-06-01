/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
 *  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de
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

#ifndef _KYZIS_EDITOR_H_
#define _KYZIS_EDITOR_H_

#include <QList>

#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>
#include <ktexteditor/configpage.h>

class KYZisEditor : public KTextEditor::Editor {
	Q_OBJECT
public:
	virtual ~KYZisEditor();

	static KTextEditor::Editor* self();

	virtual KTextEditor::Document* createDocument( QObject *parent ); 
	virtual const QList<KTextEditor::Document*>& documents();
	virtual const KAboutData* aboutData() const;
	virtual void readConfig( KConfig *config=0 );
	virtual void writeConfig( KConfig *config=0 );

	virtual bool configDialogSupported() const;
	virtual void configDialog( QWidget *parent );
	virtual int configPages() const;
	virtual KTextEditor::ConfigPage* configPage( int number, QWidget *parent );
	virtual QString configPageName( int number ) const;
	virtual QString configPageFullName( int number ) const;
	virtual KIcon configPageIcon( int number ) const;

private:
	KYZisEditor (QObject *parent);

	static KYZisEditor* me;

	QList<KTextEditor::Document*> m_documents;
};

#endif
