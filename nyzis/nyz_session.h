#ifndef NYZ_SESSION_H
#define NYZ_SESSION_H
/**
 * NYZSession - Ncurses main class for the whole session
 */

#include "yz_session.h"
#include "nyz_view.h"
#include <curses.h>


#define NYZ_VIEW_MAX 300
//#define NYZ_BUFFER_MAX 300

class NYZSession : public YZSession {
	public:
		/**
		 * Constructor. Give a session name to identify/save/load sessions.
		 */
		NYZSession( int argc, char **charv, const char *_session_name = "default_session" );

		void		event_loop();
		void		update_status(const QString& msg);

		void		save_cursor(void);
		void		restore_cursor(void);

	private:
		WINDOW		*screen;	// whole (ncurses) screen (== stdscr)

		WINDOW		*statusbar;	// the one we type command in (:wq)
		WINDOW		*infobar;	// the white one with filename/size/position...

		WINDOW		*windows[NYZ_VIEW_MAX];
		int		windows_nb;

		int		save_cursor_x; /** only to be used by save/resore _cursor() until further notice */
		int		save_cursor_y;

};

#endif // NYZ_SESSION_H

