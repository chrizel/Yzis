#ifndef NYZ_VIEW_H
#define NYZ_VIEW_H
/**
 * nyz_view.h
 *
 * ncurses-based GUI for yzis
 *
 */

#include <curses.h>
#include "yz_view.h"


#define	YZ_MAX_LINE	3000

class NYZSession;

class NYZView : public YZView  {
public:
	/**
	  * constructor. Each view is binded to a buffer, @param lines is the initial number of lines that
	  * this view can display
	  */
	NYZView(NYZSession *_session, WINDOW *_window, YZBuffer *b, int lines_vis);

	void event_loop();
	void flush_events();


protected:
	void handle_event(yz_event *e);

	YZLine		*local_lines[YZ_MAX_LINE];
	WINDOW		*window;	/* ncurses window to write to */
	NYZSession	*session;

	int		h, w;		/** height and width of the window */

private:
	void update_info(void) { getmaxyx(window, h, w); }

};


#endif // NYZ_VIEW_H
