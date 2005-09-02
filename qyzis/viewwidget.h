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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef QYZISVIEW_H
#define QYZISVIEW_H

#include <QStatusBar>
#include <qevent.h>
#include <qscrollbar.h>
#include <qlayout.h>
#include "cursor.h"
#include "editor.h"
#include "commandwidget.h"
#include "view.h"

class QYZisEdit;
class YZBuffer;
class QLabel;
class QYZisCommand;
class QYZisCodeCompletion;

class QYZisView: public QWidget, public YZView
{
	Q_OBJECT

	friend class QYZisFactory;

	signals :
		void cursorPositionChanged();
		void newStatus();

	public:
		QYZisView(YZBuffer  *doc, QWidget *parent, const char *name=0);
	 	virtual ~QYZisView();
//		KTextEditor::Document *document () { return dynamic_cast<KTextEditor::Document*>( buffer ); }
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
//		void contextMenuEvent( QContextMenuEvent * e );

		void applyConfig( bool refresh = true );

		// return string and char width in pixels
		unsigned int stringWidth( const QString& str ) const;
		unsigned int charWidth( const QChar& ch ) const;
		QChar currentChar() const;

		QYZisEdit *editor() { return m_editor; }

		/*
		// KTextEditor::View
		bool removeSelection();
		bool removeSelectionText();
		const KTextEditor::Cursor& selectionStart() const;
		const KTextEditor::Cursor& selectionEnd() const;
		enum KTextEditor::View::EditMode viewEditMode() const {return KTextEditor::View::EditInsert;} //FIXME
		bool setSelection(const KTextEditor::Range&);
		bool hasSelection() const;

		// KTextEditor::SelectionInterface
		bool clearSelection();
		const KTextEditor::Range& selection() const;
		QString selectionText() const;
		bool removeSelectedText();
		bool selectAll();
		int selectionStartLine() const;
		int selectionStartColumn() const;
		int selectionEndLine() const;
		int selectionEndColumn() const;
		bool blockSelection() const { return ( modePool()->currentType() == YZMode::MODE_VISUAL_BLOCK ); }
		bool setBlockSelection ( bool  ) { return true; } //TODO
		void copy () const {}; //TODO
		void cut() {} ; //TODO
		void paste() {}; //TODO


		//KTextEditor::MenuInterface and support functions
		void emitNewStatus();

		//KTextEditor::CodeCompletionInterface and support functions
		virtual void showArgHint (QStringList functionList, const QString &strWrapping, const QString &strDelimiter);
//		virtual void showCompletionBox (Q3ValueList< KTextEditor::CompletionEntry > complList, int offset=0, bool casesensitive=true);
		QFontMetrics editorFontMetrics();
		virtual bool registerCompletionProvider(KTextEditor::CompletionProvider * ) { return false; }
		virtual bool unregisterCompletionProvider(KTextEditor::CompletionProvider*) { return true; }
		virtual void invokeCompletion(enum KTextEditor::CompletionType type) {} //TODO
		void invokeCompletion(KTextEditor::CompletionProvider*,enum KTextEditor::CompletionType) {} //TODO
		void completionAborted() {} //TODO
		void completionDone() {} //TODO
		void completingInProgress(bool val) {m_cc_cleanup=val;}
		*/

		virtual void registerModifierKeys( const QString& keys );
		virtual void unregisterModifierKeys( const QString& keys );
		
		bool popupFileSaveAs();
		void filenameChanged();
		void highlightingChanged();

    /**
     * Get the screen coordinates of the cursor position
     * @return cursor screen coordinates
     */
//    virtual QPoint cursorPositionCoordinates () const;

	public slots:
//		void cursorPosition ( int &line, int &col) const;
//		virtual void cursorPositionReal ( int &line, int &col) const;

		void fileSave();
		void fileSaveAs();
//		void resetInfo();
		void scrollView( int );
//		void scrollLineUp();
//		void scrollLineDown();

	private:
		YZBuffer *buffer;
		bool m_customComplete;
		bool m_cc_cleanup;
		QYZisEdit *m_editor;
		QStatusBar *status;
		QYZisCommand *command;
		QScrollBar *mVScroll; //vertical scroll
		QMenu *m_popup;
		QYZisCodeCompletion *m_codeCompletion;
		QGridLayout *g ;
//		KSqueezedTextLabel *m_central;
		QLabel *m_central;
		QLabel *l_mode, *l_fileinfo, *l_linestatus;
};

#endif

