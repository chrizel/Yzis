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

#include "ktedocument.h"
#include "kteview.h"
#include "kteeditor.h"

#include <libyzis/buffer.h>

KTEDocument::KTEDocument( QObject* /*parent*/ )
{
	m_buffer = new YZBuffer();
}

KTEDocument::~KTEDocument()
{
	delete m_buffer;
}

KTextEditor::Editor* KTEDocument::editor()
{
	return KTEEditor::self();
}

KTextEditor::View* KTEDocument::createView( QWidget* /*parent*/ )
{
	KTEView* view = new KTEView(this, 0);
	return view;
}

KTextEditor::View* KTEDocument::activeView() const
{
	// TODO: implement
	return NULL;
}

const QList<KTextEditor::View*>& KTEDocument::views() const
{
	return m_views;	
}

const QString& KTEDocument::documentName () const
{
	// TODO: implement
	static QString remove_me;
	return remove_me;
}

QString KTEDocument::mimeType()
{
	// TODO: implement
	return QString();
}

bool KTEDocument::setEncoding( const QString& /*encoding*/ )
{
	// TODO: implement
	return false;
}

const QString& KTEDocument::encoding() const
{
	// TODO: implement
	static QString remove_me;
	return remove_me;
}

bool KTEDocument::documentReload()
{
	// TODO: implement
	return false;
}

bool KTEDocument::documentSave()
{
    // TODO: implement
    return false;
}

bool KTEDocument::documentSaveAs()
{
    // TODO: implement
    return false;
}

bool KTEDocument::startEditing()
{
    // TODO: implement
    return false;
}

bool KTEDocument::endEditing()
{
	// TODO: implement
	return false;
}

QString KTEDocument::text() const
{
	// TODO: implement
	return QString();
}

QString KTEDocument::text( const KTextEditor::Range& /*range*/, bool /*block*/ ) const
{
	// TODO: implement
	return QString();
}

QChar KTEDocument::character( const KTextEditor::Cursor& /*position*/ ) const
{
	// TODO: implement
	return QChar();
}

QStringList KTEDocument::textLines( const KTextEditor::Range& /*range*/, bool /*block*/ ) const
{
	// TODO: implement
	return QStringList();
}

QString KTEDocument::line( int /*line*/ ) const
{
	// TODO: implement
	return QString();
}

int KTEDocument::lines() const
{
	// TODO: implement
	return 0;
}

KTextEditor::Cursor KTEDocument::documentEnd() const
{
	// TODO: implement
	return KTextEditor::Cursor();
}

int KTEDocument::totalCharacters() const
{
	// TODO: implement
	return 0;
}

int KTEDocument::lineLength( int /*line*/ ) const
{
	// TODO: implement
	return 0;
}

bool KTEDocument::setText( const QString& /*text*/ )
{
	// TODO: implement
	return false;
}

bool KTEDocument::setText( const QStringList& /*text*/ )
{
	// TODO: implement
	return false;
}

bool KTEDocument::clear()
{
	// TODO: implement
	return false;
}

bool KTEDocument::insertText( const KTextEditor::Cursor& /*position*/, const QString& /*text*/, bool /*block*/ )
{
	// TODO: implement
	return false;
}

bool KTEDocument::insertText( const KTextEditor::Cursor& /*position*/, const QStringList& /*text*/, bool /*block*/ )
{
	// TODO: implement
	return false;
}

bool KTEDocument::removeText( const KTextEditor::Range& /*range*/, bool /*block*/ )
{
	// TODO: implement
	return false;
}

bool KTEDocument::insertLine( int /*line*/, const QString& /*text*/ )
{
	// TODO: implement
	return false;
}

bool KTEDocument::insertLines( int /*line*/, const QStringList& /*text*/ )
{
	// TODO: implement
	return false;
}

bool KTEDocument::removeLine( int /*line*/ )
{
	// TODO: implement
	return false;
}

QString KTEDocument::mode() const
{
	// TODO: implement
	return QString();
}

QString KTEDocument::highlightingMode() const
{
	// TODO: implement
	return QString();
}

QStringList KTEDocument::modes() const
{
	// TODO: implement
	return QStringList();
}

QStringList KTEDocument::highlightingModes() const
{
	// TODO: implement
	return QStringList();
}

bool KTEDocument::setMode( const QString& /*name*/ )
{
	// TODO: implement
	return false;
}

bool KTEDocument::setHighlightingMode( const QString& /*name*/ )
{
	// TODO: implement
	return false;
}

#include "ktedocument.moc"
