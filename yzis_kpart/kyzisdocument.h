/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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

#ifndef _KYZIS_DOCUMENT_H_
#define _KYZIS_DOCUMENT_H_

#include "kyziseditor.h"

#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>
#include <ktexteditor/range.h>
#include <ktexteditor/cursor.h>

class YZBuffer;

class KYZisDocument : public KTextEditor::Document {
Q_OBJECT
public:
	KYZisDocument( QObject* parent=0 );
	virtual ~KYZisDocument();

	virtual KTextEditor::Editor* editor();
	virtual KTextEditor::View* createView( QWidget *parent );
	virtual KTextEditor::View* activeView() const;
	virtual const QList<KTextEditor::View*>& views() const;
	virtual const QString& documentName() const;
	virtual QString  mimeType();
	virtual bool setEncoding( const QString &encoding );
	virtual const QString& encoding() const;
	virtual bool documentReload();

	virtual bool documentSave();
	virtual bool documentSaveAs();
	virtual bool startEditing();

	virtual bool endEditing();
	virtual QString text() const;
	virtual QString text( const KTextEditor::Range &range, bool block=false ) const;
	virtual QChar character( const KTextEditor::Cursor &position ) const;
	virtual QStringList textLines( const KTextEditor::Range &range, bool block=false ) const;
	virtual QString line( int line ) const;
	virtual int lines() const;
	virtual KTextEditor::Cursor documentEnd() const;
	virtual int totalCharacters() const;
	virtual int lineLength( int line ) const;
	virtual bool setText( const QString &text );
	virtual bool setText( const QStringList &text );
	virtual bool clear();
	virtual bool insertText( const KTextEditor::Cursor &position, const QString &text, bool block=false );
	virtual bool insertText( const KTextEditor::Cursor &position, const QStringList &text, bool block=false );
	virtual bool removeText( const KTextEditor::Range &range, bool block=false );
	virtual bool insertLine( int line, const QString &text );
	virtual bool insertLines( int line, const QStringList &text );
	virtual bool removeLine( int line );
	virtual QString mode() const;
	virtual QString highlightingMode() const;
	virtual QStringList modes() const;
	virtual QStringList highlightingModes() const;
	virtual bool setMode( const QString &name );
	virtual bool setHighlightingMode( const QString &name );

	virtual bool openFile() { return false; }
	virtual bool saveFile() { return false; }

	YZBuffer* buffer() const { return m_buffer; }
signals:
	void   viewCreated (KTextEditor::Document *document, KTextEditor::View *view);
	void   documentNameChanged (KTextEditor::Document *document);
	void   documentUrlChanged (KTextEditor::Document *document);
	void   modifiedChanged (KTextEditor::Document *document);
	void   textChanged (KTextEditor::Document *document);
	void   textInserted (KTextEditor::Document *document, const KTextEditor::Range &range);
	void   textRemoved (KTextEditor::Document *document, const KTextEditor::Range &range);
	void   textChanged (KTextEditor::Document *document, const KTextEditor::Range &oldRange, const KTextEditor::Range &newRange);
	void   aboutToClose (KTextEditor::Document *document);
	void   modeChanged (KTextEditor::Document *document);
	void   highlightingModeChanged (KTextEditor::Document *document);

private:
	QList<KTextEditor::View*> m_views;
	YZBuffer* m_buffer;
};

#endif
