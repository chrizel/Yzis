/**
 * nyzis.h
 *
 * ncurses-based GUI for yzis
 *
 */

#include <curses.h>
#include "yz_view.h"


#define	YZ_MAX_LINE	3000

class NYZView : public YZView  {
public:
	/**
	  * constructor. Each view is binded to a buffer, @param lines is the initial number of lines that
	  * this view can display
	  */
	NYZView(YZBuffer *b, int lines) : YZView(b,lines) { }

	void event_loop();
	void flush_events();

protected:
	void handle_event(yz_event *e);
	YZLine	*lines[YZ_MAX_LINE];

};

