/**
 * $Id: nyz_view.h,v 1.10 2003/04/25 12:45:33 mikmak Exp $
 */
#ifndef NYZ_VIEW_H
#define NYZ_VIEW_H
/**
 * ncurses-based GUI for yzis
 */

#include "yz_view.h"
#include "gui.h"
#include <curses.h>
#include <qstringlist.h>
#include <qmap.h>

class NYZSession;

class NYZView : public YZView, public Gui  {
public:
	/**
	  * constructor. Each view is binded to a buffer, @param lines is the initial number of lines that
	  * this view can display
	  */
	NYZView(NYZSession *_session, WINDOW *_window, YZBuffer *b, int lines_vis);
	virtual ~NYZView();

	void event_loop();
	void flush_events();

	void postEvent(yz_event);
	YZSession *getCurrentSession();

	void scrollDown(int lines=1);
	void scrollUp(int lines=1);

protected:
	void handle_event(yz_event e);

	WINDOW		*window;	/* ncurses window to write to */
	NYZSession	*session;

	unsigned int	h, w;		/** height and width of the window */

private:
	void update_info(void) { getmaxyx(window, h, w); }

};

#endif // NYZ_VIEW_H
