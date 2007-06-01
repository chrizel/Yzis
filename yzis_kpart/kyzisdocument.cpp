/* This file is part of the Yzis libraries
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

#include "kyzisdocument.h"
#include "kyzisview.h"

KYZisDocument::KYZisDocument( QObject* /*parent*/ )
{

}

KYZisDocument::~KYZisDocument()
{

}

KTextEditor::Editor* KYZisDocument::editor()
{
	return KYZisEditor::self();
}

KTextEditor::View* KYZisDocument::createView( QWidget* /*parent*/ )
{
	KYZisView* view = new KYZisView(this, 0);
	return view;
}

KTextEditor::View* KYZisDocument::activeView() const
{
	// TODO: implement
	return NULL;
}

const QList<KTextEditor::View*>& KYZisDocument::views() const
{
	return m_views;	
}

const QString& KYZisDocument::documentName () const
{
	// TODO: implement
	static QString remove_me;
	return remove_me;
}

QString KYZisDocument::mimeType()
{
	// TODO: implement
	return QString();
}

bool KYZisDocument::setEncoding( const QString& /*encoding*/ )
{
	// TODO: implement
	return false;
}

const QString& KYZisDocument::encoding() const
{
	// TODO: implement
	static QString remove_me;
	return remove_me;
}

bool KYZisDocument::documentReload()
{
	// TODO: implement
	return false;
}

bool KYZisDocument::documentSave()
{
    // TODO: implement
    return false;
}

bool KYZisDocument::documentSaveAs()
{
    // TODO: implement
    return false;
}

bool KYZisDocument::startEditing()
{
    // TODO: implement
    return false;
}

bool KYZisDocument::endEditing()
{
	// TODO: implement
	return false;
}

QString KYZisDocument::text() const
{
	// TODO: implement
	return QString();
}

QString KYZisDocument::text( const KTextEditor::Range& /*range*/, bool /*block*/ ) const
{
	// TODO: implement
	return QString();
}

QChar KYZisDocument::character( const KTextEditor::Cursor& /*position*/ ) const
{
	// TODO: implement
	return QChar();
}

QStringList KYZisDocument::textLines( const KTextEditor::Range& /*range*/, bool /*block*/ ) const
{
	// TODO: implement
	return QStringList();
}

QString KYZisDocument::line( int /*line*/ ) const
{
	// TODO: implement
	return QString();
}

int KYZisDocument::lines() const
{
	// TODO: implement
	return 0;
}

KTextEditor::Cursor KYZisDocument::documentEnd() const
{
	// TODO: implement
	return KTextEditor::Cursor();
}

int KYZisDocument::totalCharacters() const
{
	// TODO: implement
	return 0;
}

int KYZisDocument::lineLength( int /*line*/ ) const
{
	// TODO: implement
	return 0;
}

bool KYZisDocument::setText( const QString& /*text*/ )
{
	// TODO: implement
	return false;
}

bool KYZisDocument::setText( const QStringList& /*text*/ )
{
	// TODO: implement
	return false;
}

bool KYZisDocument::clear()
{
	// TODO: implement
	return false;
}

bool KYZisDocument::insertText( const KTextEditor::Cursor& /*position*/, const QString& /*text*/, bool /*block*/ )
{
	// TODO: implement
	return false;
}

bool KYZisDocument::insertText( const KTextEditor::Cursor& /*position*/, const QStringList& /*text*/, bool /*block*/ )
{
	// TODO: implement
	return false;
}

bool KYZisDocument::removeText( const KTextEditor::Range& /*range*/, bool /*block*/ )
{
	// TODO: implement
	return false;
}

bool KYZisDocument::insertLine( int /*line*/, const QString& /*text*/ )
{
	// TODO: implement
	return false;
}

bool KYZisDocument::insertLines( int /*line*/, const QStringList& /*text*/ )
{
	// TODO: implement
	return false;
}

bool KYZisDocument::removeLine( int /*line*/ )
{
	// TODO: implement
	return false;
}

QString KYZisDocument::mode() const
{
	// TODO: implement
	return QString();
}

QString KYZisDocument::highlightingMode() const
{
	// TODO: implement
	return QString();
}

QStringList KYZisDocument::modes() const
{
	// TODO: implement
	return QStringList();
}

QStringList KYZisDocument::highlightingModes() const
{
	// TODO: implement
	return QStringList();
}

bool KYZisDocument::setMode( const QString& /*name*/ )
{
	// TODO: implement
	return false;
}

bool KYZisDocument::setHighlightingMode( const QString& /*name*/ )
{
	// TODO: implement
	return false;
}

#include "kyzisdocument.moc"
