/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
 *  Copyright (C) 2003-2004 Lucijan Busch <lucijan@kde.org>
 *  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
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

#include "kteeditor.h"
#include "ktedocument.h"
#include "kyzissession.h"


#include <debug.h>

// --------------------------------  static variables

KTEEditor* KTEEditor::me = 0;

// --------------------------------  methods

KTEEditor::KTEEditor(QObject* parent)
	: KTextEditor::Editor(parent)
{
}

KTEEditor::~KTEEditor()
{

}

KTextEditor::Editor* KTEEditor::self()
{
	if (!me) {
		me = new KTEEditor(0);
	}
	return me;
}

KTextEditor::Document* KTEEditor::createDocument( QObject *parent )
{
	KTEDocument* doc = new KTEDocument(parent);
	emit documentCreated( this, doc );
	return doc;
}

const QList<KTextEditor::Document*>& KTEEditor::documents() 
{
	// TODO: create mechanism to register/unregister documents
	return m_documents;
}

const KAboutData* KTEEditor::aboutData () const 
{
	// TODO: implement
	return 0;
}

void KTEEditor::readConfig (KConfig* /*config*/)
{
	// TODO: implement
}

void KTEEditor::writeConfig( KConfig* /*config*/ )
{
	// TODO: implement
}

bool KTEEditor::configDialogSupported() const
{
	// TODO: implement
	return false;
}

void KTEEditor::configDialog( QWidget* /*parent*/ ) 
{
	// TODO: implement
}

int KTEEditor::configPages() const 
{
	// TODO: implement
	return 0;
}

KTextEditor::ConfigPage* KTEEditor::configPage( int /*number*/, QWidget* /*parent*/ ) 
{
	// TODO: implement
	return 0;
}

QString KTEEditor::configPageName( int /*number*/ ) const  
{
	// TODO: implement
	return QString();
}

QString KTEEditor::configPageFullName( int /*number*/ ) const 
{
	// TODO: implement
	return QString();
}

KIcon KTEEditor::configPageIcon( int /*number*/ ) const 
{
	// TODO: implement
	return KIcon();
}

#include "kteeditor.moc"
