/* This file is part of the Yzis libraries
 *  Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>
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

#ifndef KYZISVIEW_H
#define KYZISVIEW_H

#include <ktexteditor/view.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/codecompletioninterface.h>
#include <kstatusbar.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qlayout.h>
#include "cursor.h"
#include "document.h"
#include "editor.h"
#include "viewwidget.h"
#include "commandwidget.h"

class KYZisEdit;
class KYZisCommand;
class KYZisCodeCompletion;

class KYZisView: public KTextEditor::View, public KTextEditor::ViewCursorInterface, public KTextEditor::PopupMenuInterface, public KTextEditor::CodeCompletionInterface, public YZView
{
	Q_OBJECT

	friend class KYZisFactory;

	signals :
		void cursorPositionChanged();
		void newStatus();

		//KTextEditor::CodeCompletionInterface signals
		void completionAborted();
		void completionDone();
		void completionDone(KTextEditor::CompletionEntry);
		void argHintHidden();
		void filterInsertString(KTextEditor::CompletionEntry *, QString *);

	public:
		KYZisView(KYZisDoc *doc, QWidget *parent, const char *name=0);
	 	virtual ~KYZisView();
		KTextEditor::Document *document () const { return buffer; }
		void setCommandLineText( const QString& text );
		QString getCommandLineText() const;
		void setFocusCommandLine();
		void setFocusMainWindow();
		void scrollDown( int l=1 );
		void scrollUp( int l=1 );

		void paintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh );
		virtual void modeChanged(void);
		virtual void syncViewInfo();
		void displayInfo( const QString& info );
		void refreshScreen ();

		/**
		 * @internal
		 * Sets the Kmdi Id of this view, used for DCOP communications
		 */
		void setkid( int kId ) { mkId = kId; }

		void KYZisView::wheelEvent( QWheelEvent * e );
		void KYZisView::contextMenuEvent( QContextMenuEvent * e );

		/**
		 * @internal
		 * Get the Kmdi Id of this view
		 */
		int getkid() { return mkId; }

		void applyConfig( bool refresh = true );

		// return string and char width in pixels
		unsigned int stringWidth( const QString& str ) const;
		unsigned int charWidth( const QChar& ch ) const;

		KYZisEdit *editor() { return m_editor; }

		//KTextEditor::PopupMenuInterface and support functions
		virtual void installPopup( QPopupMenu *rmb_Menu );
		void emitNewStatus();

		//KTextEditor::CodeCompletionInterface and support functions
		virtual void showArgHint (QStringList functionList, const QString &strWrapping, const QString &strDelimiter);
		virtual void showCompletionBox (QValueList< KTextEditor::CompletionEntry > complList, int offset=0, bool casesensitive=true);
		QFontMetrics editorFontMetrics();

	public slots:
		QPoint cursorCoordinates();
		void cursorPosition ( unsigned int *line, unsigned int *col);
		virtual void cursorPositionReal ( unsigned int *line, unsigned int *col);
		bool setCursorPosition ( unsigned int line, unsigned int col);
		bool setCursorPositionReal(unsigned int line, unsigned int col);

		unsigned int cursorLine();
		unsigned int cursorColumn();
		unsigned int cursorColumnReal();
		void fileSave();
		void fileSaveAs();
		void resetInfo();
		void scrolled( int );

	protected:
		void setupActions();
		void setupCodeCompletion();
		virtual void registerModifierKeys( const QString& keys );

	private:
		KYZisEdit *m_editor;
		KYZisDoc *buffer;
		KStatusBar *status;
		KYZisCommand *command;
		QScrollBar *mVScroll; //vertical scroll
		QPopupMenu *m_popup;
		KYZisCodeCompletion *m_codeCompletion;
		QGridLayout *g ;
		// internal ID used for DCOP communications
		int mkId;
};

#endif
