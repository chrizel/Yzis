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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/**
 * $Id$
 */
#ifndef NYZ_VIEW_H
#define NYZ_VIEW_H
/**
 * ncurses-based GUI for yzis
 */

#include "view.h"
#include "cursor.h"
#include <ncurses.h>
#if QT_VERSION < 0x040000
#include <qstringlist.h>
#include <qmap.h>
#include <qnamespace.h>
#else
#include <QMap>
#include <QDataStream>
#include <QStringList>
#endif

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
	void paintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh );

	void scrollUp( int );
	void scrollDown( int );

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

protected :

public slots:

	protected  :

		inline unsigned int stringWidth( const QString& /*str*/ ) const {
			return 1;
		}
		inline unsigned int charWidth( const QChar& /*ch*/ ) const {
			return 1;
		}


private:
	WINDOW		*window;	/* ncurses window to write to */
	/**
	 * update visible area
	 */
	void updateVis( bool refresh=true );
	/**
	  * Display a line
	  * @arg line is the line number, taken from the beginning of the file ( and not
	  * the beginning of the displayed part )
	  */
	void drawContents( int , int );
	/**
	  * print a void line ( that is "~    " )
	  * @arg line is the line # relative to the displayed screen, not the buffer ( as this
	  * doesn't belong to the buffer anyway..)
	  */
	void printVoid( unsigned int line );

	/**
	  * draw cursor
	  */
	void drawCursor();

	/* layout */
	WINDOW		*editor;
	WINDOW		*infobar;	// the white one with filename/size/position...
	WINDOW		*statusbar;	// the one we show in which mode we are
	WINDOW          *fileInfo;     // the one with info about current file (modified..)

	/**
	  * used to implement set/get CommandLine, as we have no
	  * special widget for that
	  */
	QString commandline;
	unsigned int lastLineNumber;
	unsigned int marginLeft;
	unsigned int height;
	unsigned int width;

	void initialiseAttributesMap();
	static int attributesMapInitialised;
	/**
	  * maps QRgb to ncurses attributes, as those used in
	  * mAttributesMap[ QRgb ] is the # of the corresponding pair in ncurses
	  */
	static QMap<QRgb,unsigned long int> mAttributesMap;
};

#endif // NYZ_VIEW_H



