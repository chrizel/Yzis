#ifndef YZ_VIEW_H
#define YZ_VIEW_H
/**
 * $id$
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
	yz_event fetchNextEvent();

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

	YZBuffer *myBuffer() { return buffer;}

	YZSession *mySession() { return session; }
	
	/**
	 * Register a GUI event manager
	 */
	void registerManager ( Gui *mgr );

	void updateCursor(int x=-1,int y=-1);

	void centerView( int line );

	void purgeInputBuffer() { previous_chars="";}

	/**
	 * moves the cursor of the current view down
	 */
	QString moveDown( QString inputsBuff = QString::null );

	/**
	 * moves the cursor of the current view up 
	 */
	QString moveUp( QString inputsBuff = QString::null );

	/**
	 * moves the cursor of the current view left
	 */
	QString moveLeft( QString inputsBuff = QString::null );

	/**
	 * moves the cursor of the current view right
	 */
	QString moveRight( QString inputsBuff = QString::null );

	/**
	 * Start insert mode
	 */
	QString gotoInsertMode( QString inputsBuff = QString::null );

	/**
	 * Start replace mode
	 */
	QString gotoReplaceMode( QString inputsBuff = QString::null );

protected:
	YZBuffer 	*buffer; 	/** buffer we use */
	int		lines_vis;	/** number of visible lines */
	int		current;	/** current line on top of view */
	YZCursor *cursor;
	int		current_maxx;	/** maximum value for x, that s (lenght of current line -1), -1 means the line is empty */

	enum {
		YZ_VIEW_MODE_INSERT,
		YZ_VIEW_MODE_REPLACE,
		YZ_VIEW_MODE_COMMAND 
	}		mode;		/** mode of this view */


	/* fifo event loop. really basic now */
	//FIXME replace with a QMap
	QValueList<yz_event> events;
//	yz_event	events[YZ_EVENT_EVENTS_MAX];
//	int		events_nb_begin;
//	int		events_nb_last;

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

private:
	/**
	 * The current session, provided by the GUI
	 */
	YZSession *session;

};

#endif /*  YZ_VIEW_H */

