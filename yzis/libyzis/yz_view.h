#ifndef YZ_VIEW_H
#define YZ_VIEW_H
/**
 * YZView - abstraction of a view. binded to a buffer of course
 */


#include "yz_events.h"
#include "yz_buffer.h"
#include "event_mgr.h"

#define YZ_EVENT_EVENTS_MAX 400

class YZView {
public:
	/**
	  * constructor. Each view is binded to a buffer, @param lines is the initial number of lines that
	  * this view can display
	  */
	YZView(YZBuffer *_b, int _lines_vis);

	/**
	 * transfer a key event from gui to core
	 */
	void	send_char( QChar );

	/**
	  * Used by the buffer to post events
	  */
	void	post_event (yz_event );
	/**
	 * get a event to handle from the core.
	 * that's the way the core is sending messages to the gui
	 */
	/* for the qt/kde gui, we should create QEvents from that? */
	yz_event *	fetch_event(int idx=-1);

	/**
	  * returns the number of the line displayed on top of this view
	  * (refering to the whole file/buffer
	  */
	int	get_current(void) { return current; }

	/**
	  * returns the number of line this view can display
	  */
	int	get_lines_visible(void) { return lines_vis; }

	/**
	  * return true or false according to if the given line is
	  * visible or not
	  */
	int	is_line_visible(int l) { return ( (l>=current) && ((l-current)<lines_vis) ); }
	
	/**
	 * Register a GUI event manager
	 */
	void register_manager ( EventMgr *mgr );

protected:
	void	update_cursor(void);

protected:
	YZBuffer 	*buffer; 	/** buffer we use */
	int		lines_vis;	/** number of visible lines */
	int		current;	/** current line on top of view */
	int		cursor_x;	/** current cursor position, relative to the whole file */
	int		cursor_y;
	int		cursor_x_ghost;	/** the one we would be if the line was long enough */
	int		current_maxx;	/** maximum value for x, that s (lenght of current line -1), -1 means the line is empty */
	enum {
		YZ_VIEW_MODE_INSERT,
		YZ_VIEW_MODE_REPLACE,
		YZ_VIEW_MODE_COMMAND 
	}		mode;		/** mode of this view */


	/* fifo event loop. really basic now */
	yz_event	events[YZ_EVENT_EVENTS_MAX];
	int		events_nb_begin;
	int		events_nb_last;

	/*
	 * mode handling (on a per-view basis, no?)
	 * most of command handling and drawing stuff...
	 * yzview should update the core as to how many lines it can display (changegeometry stuff)
	 */

	/**
	 * Current view manager
	 */
	EventMgr *gui_manager;
};

#endif /*  YZ_VIEW_H */

