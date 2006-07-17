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

class QSettings;

class QYZisView: public QWidget, public YZView
{
	Q_OBJECT

	friend class QYZisFactory;
	friend class QYZisCursor;
	friend class QYZisEdit;

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
		void scroll( int dx, int dy );

		virtual void modeChanged(void);
		virtual void syncViewInfo();
		void displayInfo( const QString& info );

		void wheelEvent( QWheelEvent * e );
//		void contextMenuEvent( QContextMenuEvent * e );

		void applyConfig( const QSettings& settings, bool refresh = true );

		// return string and char width in pixels
		int stringWidth( const QString& str ) const;
		int charWidth( const QChar& ch ) const;
		QChar currentChar() const;

		QYZisEdit *editor() { return m_editor; }

		virtual void registerModifierKeys( const QString& keys );
		virtual void unregisterModifierKeys( const QString& keys );

		void refreshScreen();
		
		bool popupFileSaveAs();
		void filenameChanged();
		void highlightingChanged();
	
	protected:
		void drawSetMaxLineNumber( int max );
		void drawSetLineNumber( int y, int n, int h );
		virtual void preparePaintEvent( int y_min, int y_max );
		virtual void endPaintEvent();
		virtual void drawCell( int x, int y, const YZDrawCell& cell, void* arg );
		virtual void drawClearToEOL( int x, int y, const QChar& clearChar );

		void paintEvent( const YZSelection& s );


		virtual void notifyContentChanged( const YZSelection& i );

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
		QLabel *m_central;
		QLabel *l_mode, *l_fileinfo, *l_linestatus;

		QVBoxLayout* m_lineNumber;

		QPainter* m_painter;
};

#endif

