#ifndef NYZ_SESSION_H
#define NYZ_SESSION_H
/**
 * NYZSession - Ncurses main class for the whole session
 */

#include "curses.h"
#include "yz_session.h"
#include "nyz_view.h"


#define NYZ_VIEW_MAX 300
#define NYZ_BUFFER_MAX 300

class NYZSession : public YZSession {
	public:
		/**
		 * Constructor. Give a session name to identify/save/load sessions.
		 */
		NYZSession( int argc, char **charv, const char *_session_name = "default_session" );

		void		event_loop();
		void		update_status(char *msg);

	protected:
#if 1
		virtual		YZBuffer *buffer(int i) {return buffers[i]; }
		virtual		YZView *view(int i) { return views[i]; }
#else
		YZBuffer *buffer(int i) {return buffers[i]; }
		YZView *view(int i) { return views[i]; }
#endif

		void		add_buffer( YZBuffer *b);
		void		add_view(NYZView *v);
	private:
		WINDOW		*screen;	// whole (ncurses) screen (== stdscr)

		WINDOW		*statusbar;	// the one we type command in (:wq)
		WINDOW		*infobar;	// the white one with filename/size/position...

		WINDOW		*windows[NYZ_VIEW_MAX];
		int		windows_nb;

		YZBuffer	*buffers[NYZ_BUFFER_MAX];
		NYZView		*views[NYZ_VIEW_MAX];


};

#endif // NYZ_SESSION_H

