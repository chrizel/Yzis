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

#ifndef NYZ_FACTORY_H
#define NYZ_FACTORY_H
/**
 * NYZFactory - Ncurses main class for the whole session
 */

#include "viewwidget.h"
#include <curses.h>
#include "factory.h"
#include "session.h"
#include "document.h"


#define STATUSBARWIDTH 15
#define NYZ_VIEW_MAX 300
//#define NYZ_BUFFER_MAX 300

class NYZFactory : public YZSession {
public:
	static NYZFactory *self;
	static NYZView *currentView;

public:
	/**
	 * Constructor. Give a session name to identify/save/load sessions.
	 */
	NYZFactory( int argc, char **charv, const char *session_name = "default_nyzis_session" );
	virtual ~NYZFactory( );

	void		event_loop();
	void		update_infobar(int, int, int, const QString& msg);
	QString getCommandLine() const;
	void setCommandLine( const QString& );


	virtual void scrollDown(int lines=1);
	virtual void scrollUp(int lines=1);

	virtual void quit ( bool savePopup=true ) ;
	virtual void changeCurrentView ( YZView * );
	virtual YZView* createView( YZBuffer* );
	virtual	NYZisDoc *createBuffer(const QString& path=QString::null);
	virtual void popupMessage( const QString& message );
	virtual void deleteView();

	void setStatusText( const QString& );

private:
	
private:
	/* design
	 * ------------------ infobar ---------------------
	 * statusbar |     command 
	 */

	WINDOW		*screen;	// whole (ncurses) screen (== stdscr)

	WINDOW		*infobar;	// the white one with filename/size/position...
	WINDOW          *commandbar;   // the one we type command in (:wq..)
	WINDOW		*statusbar;	// the one we show in which mode we are

	//XXX QMap-me ;)
	WINDOW		*windows[NYZ_VIEW_MAX];
	int		windows_nb;
	QString commandline;

	/**
	 * Fill the map of keycodes -> Ncurses to Qt
	 */
	void initialiseKeycodes();

	//XXX static ?
	QMap<int,Qt::Key> keycodes; // map Ncurses to Qt codes



};

#endif // NYZ_FACTORY_H

