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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KYZISVIEW_H
#define KYZISVIEW_H

#include <ktexteditor/view.h>
#include <ktexteditor/codecompletioninterface.h>
#include <kstatusbar.h>
#include <ksqueezedtextlabel.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qlayout.h>
#include "cursor.h"
#include "ktexteditoriface.h"
#include "editor.h"
#include "commandwidget.h"
#include "view.h"

class KYZisEdit;
class KYZisCommand;
class KYZisCodeCompletion;

class KYZisView: public KTextEditor::View,
				 public KTextEditor::CodeCompletionInterface,
				 public YZView
{
	Q_OBJECT

	friend class KYZisFactory;

	signals :
		void cursorPositionChanged();
		void newStatus();

	public:
		KYZisView(KYZTextEditorIface *doc, QWidget *parent, const char *name=0);
	 	virtual ~KYZisView();
		KTextEditor::Document *document () { return dynamic_cast<KTextEditor::Document*>( buffer ); }
		void setCommandLineText( const QString& text );
		QString getCommandLineText() const;
		void setFocusCommandLine();
		void setFocusMainWindow();
		void scrollDown( int l=1 );
		void scrollUp( int l=1 );

		void paintEvent( const YZSelection& drawMap );
		virtual void modeChanged(void);
		virtual void syncViewInfo();
		void displayInfo( const QString& info );

		void wheelEvent( QWheelEvent * e );
		void contextMenuEvent( QContextMenuEvent * e );

		void applyConfig( bool refresh = true );

		// return string and char width in pixels
		unsigned int stringWidth( const QString& str ) const;
		unsigned int charWidth( const QChar& ch ) const;
		QChar currentChar() const;

		KYZisEdit *editor() { return m_editor; }

		// KTextEditor::View
		bool removeSelection();
		bool removeSelectionText();
		const KTextEditor::Cursor& selectionStart() const;
		const KTextEditor::Cursor& selectionEnd() const;
		enum KTextEditor::View::EditMode viewEditMode() const {return KTextEditor::View::EditInsert;} //FIXME
		bool setSelection(const KTextEditor::Range&);
		bool clearSelection();
		bool selection() const;
		const KTextEditor::Range& KYZisView::selectionRange() const;
		QString selectionText() const;
		bool removeSelectedText();
		bool selectAll();
		int selectionStartLine() const;
		int selectionStartColumn() const;
		int selectionEndLine() const;
		int selectionEndColumn() const;
		bool blockSelection() const { return ( modePool()->currentType() == YZMode::MODE_VISUAL_BLOCK ); }
		bool setBlockSelection ( bool /*on*/ ) { return true; } //TODO
		void copy () const {}; //TODO
		void cut() {} ; //TODO
		void paste() {}; //TODO


		//KTextEditor::MenuInterface and support functions
		void emitNewStatus();

		//KTextEditor::CodeCompletionInterface and support functions
		virtual void showArgHint (QStringList functionList, const QString &strWrapping, const QString &strDelimiter);
//		virtual void showCompletionBox (Q3ValueList< KTextEditor::CompletionEntry > complList, int offset=0, bool casesensitive=true);
		QFontMetrics editorFontMetrics();
		virtual bool registerCompletionProvider(KTextEditor::CompletionProvider */*provider*/) { return false; }
		virtual bool unregisterCompletionProvider(KTextEditor::CompletionProvider*) { return true; }
		virtual void invokeCompletion(enum KTextEditor::CompletionType type) {} //TODO
		void invokeCompletion(KTextEditor::CompletionProvider*,enum KTextEditor::CompletionType) {} //TODO
		void completionAborted() {} //TODO
		void completionDone() {} //TODO
		void completingInProgress(bool val) {m_cc_cleanup=val;}

		virtual void registerModifierKeys( const QString& keys );
		virtual void unregisterModifierKeys( const QString& keys );
		
		bool popupFileSaveAs();
		void filenameChanged();
		void highlightingChanged();
		
		void setKPart( KParts::ReadWritePart *part ) { m_part = part; }
		KParts::ReadWritePart *getKPart() const { return m_part; }

  // KTextEditor::View Stuff
  public:
    /**
     * Current view state
     * This can be used for example to show up that this view is now
     * in INSERT mode, or OVERWRITE mode, or COMMAND mode, or whatever
     * The string should be i18n, as this is a user aimed representation
     * of the view state, which should be shown in the GUI
     */
    virtual QString viewMode () const { return i18n ("Normal"); }

    /**
     * Set a context menu for this view
     * @param menu new context menu object for this view
     */
    virtual void setContextMenu ( QMenu *menu );

    /**
     * Retrieve the context menu for this view
     * @return context menu object for this view or 0
     */
    virtual QMenu *contextMenu ();

     /**
     * Set the cursor position, position is in characters
     * @param position new cursor position
     * @return success
     */
    virtual bool setCursorPosition (const KTextEditor::Cursor &position);

    /**
     * Get the cursor position, position is in characters
     * @return cursor position
     */
    virtual const KTextEditor::Cursor &cursorPosition () const;

    /**
     * Get the virtual cursor position
     * @return cursor position, tabs count as MULTIPLE chars, as configured by user
     * this allows access to the user visible values of the cursor position
     */
    virtual KTextEditor::Cursor cursorPositionVirtual () const;

    /**
     * Get the screen coordinates of the cursor position
     * @return cursor screen coordinates
     */
    virtual QPoint cursorPositionCoordinates () const;

	public slots:
		void cursorPosition ( int &line, int &col) const;
		virtual void cursorPositionReal ( int &line, int &col) const;

		void fileSave();
		void fileSaveAs();
		void resetInfo();
		void scrollView( int );
		void scrollLineUp();
		void scrollLineDown();

	protected:
		void setupActions();
		void setupCodeCompletion() {} //TODO
	signals:
		void gotFocus(  KTextEditor::View* );
		void lostFocus(  KTextEditor::View* );
		void selectionChanged( KTextEditor::View* );

	private:
		QLinkedList<KTextEditor::CompletionProvider*> m_completionProviders;
		QHash<KTextEditor::CompletionProvider*,KTextEditor::CompletionData> m_completionProviderData;
		bool m_customComplete;
		bool m_cc_cleanup;
		enum KTextEditor::CompletionType m_delayed_cc_type;
		KTextEditor::CompletionProvider* m_delayed_cc_provider;
		KYZisEdit *m_editor;
		KYZTextEditorIface *buffer;
		KStatusBar *status;
		KYZisCommand *command;
		QScrollBar *mVScroll; //vertical scroll
		QMenu *m_popup;
		KYZisCodeCompletion *m_codeCompletion;
		QGridLayout *g ;
		KSqueezedTextLabel *m_central;
		
		KParts::ReadWritePart *m_part;
};

#endif
