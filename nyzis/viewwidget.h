/*
	  Copyright (c) 2003 Yzis Team <yzis-dev@yzis.org>

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
#include <curses.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qnamespace.h>

#define KEY_ESCAPE 27
#ifdef KEY_ENTER
#undef KEY_ENTER
#define KEY_ENTER 10
#endif
#define KEY_RETURN 13

#define STATUSBARWIDTH 15

class NYZSession;

//class NYZView : public QObject, public YZView
class NYZView : public YZView
{
//	Q_OBJECT

public:
	/**
	  * constructor. Each view is binded to a buffer, @arg lines is the initial number of lines that
	  * this view can display
	  */
	NYZView(YZBuffer *b);
	virtual ~NYZView();

	virtual QString getCommandLineText(void) const;
	virtual void setCommandLineText( const QString& );
	virtual void setFocusCommandLine() {}
	virtual void setFocusMainWindow() {}
	virtual void invalidateLine ( unsigned int line );
	virtual void modeChanged(void);
	virtual void refreshScreen();
	virtual void syncViewInfo();
	virtual void displayInfo(  const QString& info );
	QString getCommandLine() const;
	void setCommandLine( const QString& );

	void map( void );
	void unmap( void );

protected:
	WINDOW		*window;	/* ncurses window to write to */
	unsigned int	h, w;		/** height and width of the window */

public slots:

private:
	void update_info(void);
	/**
	  * Display a line
	  * @arg line is the line number, taken from the beginning of the file ( and not
	  * the beginning of the displayed part )
	  */
	void printLine( int line );
	/**
	  * print a void line ( that is "~    " )
	  * @arg line is the line # relative to the displayed screen, not the buffer ( as this
	  * doesn't belong to the buffer anyway..)
	  */
	void printVoid( unsigned int line );

	WINDOW		*infobar;	// the white one with filename/size/position...
	WINDOW		*statusbar;	// the one we show in which mode we are
	WINDOW          *fileInfo;     // the one with info about current file (modified..)

	QString commandline;
	bool modeDisplayed;


	void initialisecolormap();
	static int colormapinitialised;
	/**
	  * maps QRgb to ncurses colors
	  * mColormap[ QRgb ] is the # of the corresponding pair in ncurses
	  */
	static QMap<QRgb,int> mColormap;

};

#endif // NYZ_VIEW_H



