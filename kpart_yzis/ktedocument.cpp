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
#include "kysession.h"

#include <libyzis/buffer.h>

KTEDocument::KTEDocument( QObject* /*parent*/ )
{
    m_buffer = KYSession::self()->createBuffer();
}

KTEDocument::~KTEDocument()
{
    delete m_buffer;
}

KTextEditor::Editor* KTEDocument::editor()
{
    return KTEEditor::self();
}

KTextEditor::View* KTEDocument::createView( QWidget* parent )
{
    KTEView* view = new KTEView( this, parent );
    emit viewCreated( this, view );
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

bool KTEDocument::openFile()
{
    m_buffer->clearText();
    emit textRemoved( this, documentRange() );
    m_buffer->load( localFilePath() );
    emit textInserted( this, documentRange() );
    emit documentUrlChanged( this );
    return true;
}

bool KTEDocument::saveFile()
{
    m_buffer->save();
    return true;
}

const QString& KTEDocument::documentName() const
{
    return m_buffer->fileName();
}

QString KTEDocument::mimeType()
{
    // TODO: implement
    return QString();
}

bool KTEDocument::setEncoding( const QString& encoding )
{
    m_buffer->setEncoding(encoding);
    return true;
}

const QString& KTEDocument::encoding() const
{
    return m_buffer->encoding();
}

bool KTEDocument::documentReload()
{
    // TODO: implement
    return false;
}

bool KTEDocument::documentSave()
{
    return m_buffer->save();
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
    return m_buffer->getWholeText();
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

QString KTEDocument::line( int line ) const
{
    return m_buffer->textline( line );
}

int KTEDocument::lines() const
{
    return m_buffer->lineCount();
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

int KTEDocument::lineLength( int line ) const
{
    return m_buffer->getLineLength(line);
}

bool KTEDocument::setText( const QString& text )
{
    QString content = text;
    m_buffer->loadText( &content );
    return false;
}

bool KTEDocument::setText( const QStringList& text )
{
    for ( int i = 0; i < text.size(); ++i ) {
        QString content = text.at(i);
        m_buffer->loadText( &content );
    }

    return true;
}

bool KTEDocument::clear()
{
    m_buffer->clearText();
    return true;
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

bool KTEDocument::insertLine( int line, const QString& text )
{
    m_buffer->insertLine(text, line);
    return true;
}

bool KTEDocument::insertLines( int line, const QStringList& text )
{
    for ( int i = 0; i < text.size(); ++i ) {
        m_buffer->insertLine(text.at(i), line);
        ++line;
    }
    return true;
}

bool KTEDocument::removeLine( int line )
{
    m_buffer->deleteLine(line);
    return true;
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
