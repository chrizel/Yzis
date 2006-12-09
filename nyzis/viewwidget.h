/*
	Copyright (c) 2003 Thomas Capricelli <orzel@freehackers.org>
	Copyright (c) 2004-2005 Mickael Marchand <marchand@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef NYZ_VIEW_H
#define NYZ_VIEW_H
/**
 * ncurses-based GUI for yzis
 */

/* Std */
#include <ncursesw/ncurses.h>
// This is an ugly hack preventing the compiler to shock on scroll (used in
// both Qt API and (as a macro!) in ncurwses
#define curses_scroll scroll
#undef scroll

/* Qt */
#include <QMap>
#include <QDataStream>
#include <QStringList>

/* Yzis */
#include "view.h"
#include "cursor.h"

#include <drawbuffer.h>

class NYZSession;

class NYZView : public YZView {

public:
	/**
	  * constructor. Each view is binded to a buffer, @arg lines is the initial number of lines that
	  * this view can display
	  */
	NYZView(YZBuffer *b);
	virtual ~NYZView();

	virtual QString getCommandLineText(void) const {return commandline; }
	virtual void setCommandLineText( const QString& );
	virtual void modeChanged(void) { syncViewInfo(); }
	virtual void refreshScreen();
	virtual void syncViewInfo();
	virtual void displayInfo(  const QString& info );
	void paintEvent( const YZSelection& drawMap );

	void Scroll( int dx, int dy );

	/**
	  * Used when this view becomes viewable, that
	  * is on front of others
	  */
	void map( void );
	/**
	  * This view is not the front one anymore, hide it
	  */
	void unmap( void );
	virtual void registerModifierKeys( const QString& ) { }
	virtual void unregisterModifierKeys( const QString& ) { }
	
	bool popupFileSaveAs();
	void filenameChanged();
	void highlightingChanged();

	void setFocusCommandLine();
	void setFocusMainWindow();
	void restoreFocus();

protected :

public slots:

	protected  :

		inline int stringWidth( const QString& /*str*/ ) const {
			return 1;
		}
		inline int charWidth( const QChar& /*ch*/ ) const {
			return 1;
		}

		virtual void drawCell( int x, int y, const YZDrawCell& cell, void* arg );

		virtual void notifyContentChanged( const YZSelection& s );

		void preparePaintEvent(int, int);
		void endPaintEvent();
		virtual void drawClearToEOL( int x, int y, const QChar& clearChar );
		virtual void drawSetMaxLineNumber( int max );
		virtual void drawSetLineNumber( int y, int n, int h );

		bool fakeLine; /* true if current line is a fake one (eg: ~) */


private:
	WINDOW		*window;	/* ncurses window to write to */
	/**
	 * update visible area
	 */
	void updateVis( bool refresh=true );

	/**
	  * draw cursor
	  */
	void drawCursor();

	/* layout */
	WINDOW		*editor;
	WINDOW		*infobar;	// the white one with filename/size/position...
	WINDOW		*statusbar;	// the one we show in which mode we are
	WINDOW          *fileInfo;     // the one with info about current file (modified..)
	
	enum e_focusable {
		w_editor,
		w_statusbar
	};

	bool statusbarHasCommand; // true if status bar contains a command

	/**
	  * used to implement set/get CommandLine, as we have no
	  * special widget for that
	  */
	QString commandline;
		int marginLeft;
		int height;
		int width;

	void initialiseAttributesMap();
	static int attributesMapInitialised;
	/**
	  * maps QRgb to ncurses attributes, as those used in
	  * mAttributesMap[ QRgb ] is the # of the corresponding pair in ncurses
	  */
	static QMap<QRgb,unsigned long int> mAttributesMap;

	e_focusable m_focus;

};

#endif // NYZ_VIEW_H



