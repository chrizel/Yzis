#ifndef YZ_VIEW_H
#define YZ_VIEW_H
/**
 * YZView - abstraction of a view. binded to a buffer of course
 */


#include "yz_events.h"
#include "yz_buffer.h"
#include "yz_cursor.h"
#include "gui.h"

#define YZ_EVENT_EVENTS_MAX 400

class YZCursor;

class YZView {
public:
	/**
	  * constructor. Each view is binded to a buffer, @param lines is the initial number of lines that
	  * this view can display
	  */
	YZView(YZBuffer *_b, int _lines_vis);

	/**
	 * Updates the number of visible lines
	 */
	void setVisibleLines (int);

	/**
	 * transfer a key event from gui to core
	 */
	void	sendChar( QChar );

	/**
	  * Used by the buffer to post events
	  */
	void	postEvent (yz_event );

	/**
	 * get a event to handle from the core.
	 * that's the way the core is sending messages to the gui
	 */
	/* for the qt/kde gui, we should create QEvents from that? */
	yz_event *	fetchEvent(int idx=-1);

	/**
	  * returns the number of the line displayed on top of this view
	  * (refering to the whole file/buffer
	  */
	int	getCurrent(void) { return current; }

	/**
	  * returns the number of line this view can display
	  */
	int	getLinesVisible(void) { return lines_vis; }

	/**
	  * return true or false according to if the given line is
	  * visible or not
	  */
	int	isLineVisible(int l) { return ( (l>=current) && ((l-current)<lines_vis) ); }
	
	/**
	 * Register a GUI event manager
	 */
	void registerManager ( Gui *mgr );

	void updateCursor(int x=-1,int y=-1);

	void centerView( int line );

	/**
	 * moves the cursor of the current view down
	 */
	QString moveDown( QStringList );

protected:

protected:
	YZBuffer 	*buffer; 	/** buffer we use */
	int		lines_vis;	/** number of visible lines */
	int		current;	/** current line on top of view */
	YZCursor *cursor;
//	int		cursor_x;	/** current cursor position, relative to the whole file */
//	int		cursor_y;
	//m -> orzel :you consider we are wordwrapping by default!this is not true!
	//int		cursor_x_ghost;	// the one we would be if the line was long enough 
	int		current_maxx;	/** maximum value for x, that s (lenght of current line -1), -1 means the line is empty */

	enum {
		YZ_VIEW_MODE_INSERT,
		YZ_VIEW_MODE_REPLACE,
		YZ_VIEW_MODE_COMMAND 
	}		mode;		/** mode of this view */


	/* fifo event loop. really basic now */
	//FIXME replace with a QMap
	yz_event	events[YZ_EVENT_EVENTS_MAX];
	int		events_nb_begin;
	int		events_nb_last;

	/*
	 * mode handling (on a per-view basis, no?)
	 * most of command handling and drawing stuff...
	 * yzview should update the core as to how many lines it can display (changegeometry stuff)
	 */

	/**
	 * Current GUI
	 */
	Gui *gui_manager;

	/**
	 * Used to store previous keystrokes which are not recognised as a command,
	 * this should allow us to have commands like : 100g or gg etc ...
	 */
	QString previous_chars;

};

#endif /*  YZ_VIEW_H */

