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

#ifndef NYZ_SESSION_H
#define NYZ_SESSION_H
/**
 * NYZSession - Ncurses main class for the whole session
 */

#include "viewwidget.h"
#include <curses.h>
#include "factory.h"
#include "gui.h"


#define NYZ_VIEW_MAX 300
//#define NYZ_BUFFER_MAX 300

class NYZSession : public YZSession, public Gui {
	public:
		/**
		 * Constructor. Give a session name to identify/save/load sessions.
		 */
		NYZSession( int argc, char **charv, const char *_session_name = "default_session" );

		void		event_loop();
		void		update_status(const QString& msg);
		void		update_infobar(int, int, int, const QString& msg);

		/* Gui */
		virtual void postEvent(yz_event);

		virtual void scrollDown(int lines=1);
		virtual void scrollUp(int lines=1);

		virtual QString getCommandLineText(void) const;
		virtual void setCommandLineText( const QString& );
		virtual void setFocusCommandLine() {}
		virtual void setFocusMainWindow() {}
		virtual void quit ( bool savePopup=true ) ;
		virtual void setCurrentView ( YZView * );
		virtual YZView* createView( YZBuffer* );
		virtual	YZBuffer *createBuffer(const QString& path=QString::null);
		virtual void popupMessage( const QString& message );
		
	private:
		WINDOW		*screen;	// whole (ncurses) screen (== stdscr)

		WINDOW		*statusbar;	// the one we type command in (:wq)
		WINDOW		*infobar;	// the white one with filename/size/position...

		//XXX QMap-me ;)
		WINDOW		*windows[NYZ_VIEW_MAX];
		int		windows_nb;
		QString commandline;

};

#endif // NYZ_SESSION_H

