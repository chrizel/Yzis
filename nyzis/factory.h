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

