/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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

#ifndef KYZISDOC_H
#define KYZISDOC_H

#include <ktexteditor/document.h>
#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/configpage.h>
#include <ktexteditor/view.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include "buffer.h"
#include "factory.h"

class KYZTextEditorIface : public KTextEditor::Document,
		public KTextEditor::HighlightingInterface,
		public KTextEditor::MarkInterface {
	Q_OBJECT

	public:
		KYZTextEditorIface(YZBuffer *buffer, 
				 QWidget *parentWidget = 0, 
				 const char *widgetName=0, 
				 QObject *parent=0, 
				 const char *name=0);
		virtual ~KYZTextEditorIface ();

		KDocument::View *createView ( QWidget *parent );
		const QString& documentName() const { return m_buffer->fileName(); }
		QString mimeType();
		KMimeType::Ptr mimeTypeForContent();
		const QList<KDocument::View*> &views() { return _views; }
		bool setEncoding( const QString& );
		const QString& encoding() const { return m_buffer->encoding(); }
		bool documentReload();
		bool documentSave();
		bool documentSaveAs();
		QString text( const KTextEditor::Cursor&, const KTextEditor::Cursor& ) const;
		QString line( int ) const;
		bool removeText( const KTextEditor::Cursor&, const KTextEditor::Cursor& );
		bool insertLine( int, const QString& );
		bool removeLine( int );
		KTextEditor::Editor* editor() { return KYZisPublicFactory::self(); }
		
		void removeView( KTextEditor::View * v );
		QWidget *parentWidget() { return m_parent; }

		KTextEditor::View* activeView() const { return dynamic_cast<KTextEditor::View*> (m_buffer->firstView()); } //looks stupid to me ...
		KTextEditor::Cursor end() const { return KTextEditor::Cursor( ((YZBuffer*)this)->end().x(), ((YZBuffer*)this)->end().y()); }
		QString text(const KTextEditor::Range&, bool) const;
		bool insertText(const KTextEditor::Cursor&, const QString&, bool);
		bool removeText(const KTextEditor::Range&, bool);
	
		QChar character(const KTextEditor::Cursor&) const { return 0; } //TODO
		QStringList textLines(const KTextEditor::Range&, bool) const;
		KTextEditor::Cursor documentEnd() const { return KTextEditor::Cursor( ((YZBuffer*) this)->end().x(), ((YZBuffer*) this)->end().y());}
		int totalCharacters() const { return 0; } //TODO
		bool setText(const QStringList&);
		bool insertText(const KTextEditor::Cursor&, const QStringList&, bool);
		bool insertLines(int, const QStringList&);

		int lines() const { return m_buffer->lineCount(); }
		QString text() const;
		int length() const;
		bool clear() ;

		int lineLength(int line) const;
		bool insertLine( unsigned int line, const QString &s);
		bool removeLine( unsigned int line );

		QString textLine ( unsigned int line ) const;
		bool setText (  const QString &text );
		void highlightingChanged();
		bool startEditing (KTextEditor::View * = 0) { return false; } //TODO
		bool endEditing() {return false;} //TODO

		void applyConfig( );
		
		YZBuffer *getBuffer() const { return m_buffer; }
		void setBuffer( YZBuffer *buffer ) { m_buffer = buffer; }

		//HL interface
		unsigned int highlightingInterfaceNumber () const ;
		virtual unsigned int hlMode ();
		virtual bool setHlMode (unsigned int mode);
		virtual unsigned int hlModeCount ();
		virtual QString hlModeName (unsigned int mode);
		virtual QString hlModeSectionName (unsigned int mode);
		
		//mark interface
		virtual void setMarkPixmap(KTextEditor::MarkInterface::MarkTypes type, const QPixmap &pix);
		virtual void setMarkDescription(KTextEditor::MarkInterface::MarkTypes type, const QString &d);
		virtual void markChanged ( KTextEditor::Document* document, KTextEditor::Mark mark, KTextEditor::MarkInterface::MarkChangeAction action);
		virtual void marksChanged (KTextEditor::Document* document);

	public slots:
		/*state modification changes - to comply with undocumented kate features
		this overload emits stateChanged() signal from all views*/
		virtual void setModified(bool modified);

		//KTextEditor::MarkInterface slots
		uint mark( int line );
		void setMark( int line, uint markType );
		void clearMark( int line );
		void addMark( int line, uint markType );
		void removeMark( int line, uint markType );
		QHash<int, KTextEditor::Mark*> &marks();
		void clearMarks();
		void setPixmap( MarkTypes, const QPixmap & ) {} //TODO
		void setDescription( MarkTypes, const QString & ) {} //TODO
		void setMarksUserChangable( uint /*markMask*/ ) {} //TODO

		//Undo interface //KDE4 ?
		void undo ();
		void redo ();
		void clearUndo ();
		void clearRedo ();

		uint undoCount () const;
		uint redoCount () const;

		uint undoSteps () const;
		void setUndoSteps ( uint steps );

	protected:
		bool openFile();
		bool saveFile();
		void setupActions();

	public:
		void emitChars(int a, int b, const QString& c) { emit charactersInteractivelyInserted(a,b,c); }

	private:
		QList<KDocument::View*> _views;
		QHash<int, KTextEditor::Mark*> m_marks;
		QWidget *m_parent;
		YZBuffer *m_buffer;
		
		YZBuffer *buffer();

	signals:
		void hlChanged();
		void undoChanged();
		void textChanged ();
		void charactersInteractivelyInserted( int ,int ,const QString& );
		void markChanged ( KTextEditor::Mark mark, KTextEditor::MarkInterface::MarkChangeAction action );

		//KTextEditor::MarkInterface slots
		void marksChanged();

};

#endif
