/*
	  Copyright (c) 2003, 2004 Thomas Capricelli <orzel@freehackers.org> 

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

class NYZFactory : public YZSession
{

public:
	static NYZFactory *self;
	static NYZView *currentView;

public:
	/**
	 * Constructor. Give a session name to identify/save/load sessions.
	 */
	NYZFactory(const char *session_name = "default_nyzis_session" );
	virtual ~NYZFactory( );

	void		event_loop();

	/*
	 * YZSession interface :
	 */
	virtual void quit ( int errorCode ) ;
	virtual void changeCurrentView ( YZView * );
	virtual YZView* createView( YZBuffer* );
	virtual	NYZisDoc *createBuffer(const QString& path=QString::null);
	virtual void popupMessage( const QString& message );
	virtual void deleteView(int Id);
	virtual void deleteBuffer ( YZBuffer *b );
	virtual void setFocusCommandLine() {}
	virtual void setFocusMainWindow() {}

private:
	/**
	 * Fill the map of keycodes ncurses->Qt
	 */
	void initialiseKeycodes();
	/**
	  * mapping ncurses->qt for keycodes
	  */
	static QMap<int,Qt::Key> keycodes; // map Ncurses to Qt codes

};

#endif // NYZ_FACTORY_H

