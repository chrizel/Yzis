/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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
#include <kstatusbar.h>
#include <qevent.h>
#include "document.h"
#include "editor.h"
#include "viewwidget.h"
#include "commandwidget.h"

class KYZisEdit;
class KYZisCommand;

class KYZisView: public KTextEditor::View, public KTextEditor::ViewCursorInterface, public YZView
{
	Q_OBJECT

	friend class KYZisFactory;

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

		void invalidateLine ( unsigned int line );
		void setStatusBar ( const QString& text );
		virtual void syncViewInfo();
		void displayInfo( const QString& info );
		void refreshScreen ();
		QPoint calculateCursorPositionWithTabs ( unsigned int line, unsigned int col, unsigned int tabwidth );

		/**
		 * @internal
		 * Sets the Kmdi Id of this view, used for DCOP communications
		 */
		void setkid( int kId ) { mkId = kId; }

		/**
		 * @internal
		 * Get the Kmdi Id of this view
		 */
		int getkid() { return mkId; }

	public slots:
		QPoint cursorCoordinates();
		void cursorPosition ( unsigned int *line, unsigned int *col);
		virtual void cursorPositionReal ( unsigned int *line, unsigned int *col);
		bool setCursorPosition ( unsigned int line, unsigned int col);
		bool setCursorPositionReal(unsigned int line, unsigned int col);

		unsigned int cursorLine();
		unsigned int cursorColumn();
		unsigned int cursorColumnReal(); 
		void cursorPositionChanged();
		void fileSave();
		void fileSaveAs();
		void resetInfo();
	
	protected:
		void setupActions();

	private:
		KYZisEdit *editor;
		KYZisDoc *buffer;
		KStatusBar *status;
		KYZisCommand *command;
		// internal ID used for DCOP communications 
		int mkId;
};

#endif
