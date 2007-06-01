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

#include "kyziseditor.h"
#include "kyzisdocument.h"

#include <debug.h>

// --------------------------------  static variables

KYZisEditor* KYZisEditor::me = 0;

// --------------------------------  methods

KYZisEditor::KYZisEditor(QObject* parent)
	: KTextEditor::Editor(parent)
{

}

KYZisEditor::~KYZisEditor()
{

}

KTextEditor::Editor* KYZisEditor::self()
{
	if (!me) {
		me = new KYZisEditor(0);
	}
	return me;
}

KTextEditor::Document* KYZisEditor::createDocument( QObject *parent )
{
	KYZisDocument* doc = new KYZisDocument(parent);
	return doc;
}

const QList<KTextEditor::Document*>& KYZisEditor::documents() 
{
	// TODO: create mechanism to register/unregister documents
	return m_documents;
}

const KAboutData* KYZisEditor::aboutData () const 
{
	// TODO: implement
	return 0;
}

void KYZisEditor::readConfig (KConfig* /*config*/)
{
	// TODO: implement
}

void KYZisEditor::writeConfig( KConfig* /*config*/ )
{
	// TODO: implement
}

bool KYZisEditor::configDialogSupported() const
{
	// TODO: implement
	return false;
}

void KYZisEditor::configDialog( QWidget* /*parent*/ ) 
{
	// TODO: implement
}

int KYZisEditor::configPages() const 
{
	// TODO: implement
	return 0;
}

KTextEditor::ConfigPage* KYZisEditor::configPage( int /*number*/, QWidget* /*parent*/ ) 
{
	// TODO: implement
	return 0;
}

QString KYZisEditor::configPageName( int /*number*/ ) const  
{
	// TODO: implement
	return QString();
}

QString KYZisEditor::configPageFullName( int /*number*/ ) const 
{
	// TODO: implement
	return QString();
}

KIcon KYZisEditor::configPageIcon( int /*number*/ ) const 
{
	// TODO: implement
	return KIcon();
}

#include "kyziseditor.moc"
