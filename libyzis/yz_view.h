#ifndef YZ_VIEW_H
#define YZ_VIEW_H
/**
 * $Id$
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

	void updateCursor(int x=-1, int y=-1);

	void centerView( unsigned int line );

	void purgeInputBuffer() { previous_chars="";}

	/**
	 * moves the cursor of the current view down
	 */
	QString moveDown( const QString& inputsBuff = QString::null );

	/**
	 * moves the cursor of the current view up 
	 */
	QString moveUp( const QString& inputsBuff = QString::null );

	/**
	 * moves the cursor of the current view left
	 */
	QString moveLeft( const QString& inputsBuff = QString::null );

	/**
	 * moves the cursor of the current view right
	 */
	QString moveRight( const QString& inputsBuff = QString::null );

	/**
	 * moves the cursor of the current view to the start of the current line
	 */
	QString moveToStartOfLine( const QString& inputsBuff = QString::null );

	/**
	 * moves the cursor of the current view to the end of the current line
	 */
	QString moveToEndOfLine( const QString& inputsBuff = QString::null );

	/**
	 * deletes the character under the cursor
	 */
	QString deleteCharacter( const QString& inputsBuff = QString::null );

	/**
	 * Start insert mode
	 */
	QString gotoInsertMode( const QString& inputsBuff = QString::null );

	/**
	 * Start replace mode
	 */
	QString gotoReplaceMode( const QString& inputsBuff = QString::null );

	/**
	 * Go to line of file
	 */
	QString gotoLine( const QString& inputsBuff = QString::null );

	/**
	 * Deletes lines
	 */
	QString deleteLine ( const QString& inputsBuff = QString::null );

	/**
	 * Opens a new line after current line
	 */
	QString openNewLineAfter ( const QString& inputsBuff = QString::null );
	
	/**
	 * Opens a new line before current line
	 */
	QString openNewLineBefore ( const QString& inputsBuff = QString::null );

	/**
	 * Append after current character
	 */
	QString append ( const QString& inputsBuff = QString::null );

	/**
	 * Append at End of Line
	 */
	QString appendAtEOL ( const QString& inputsBuff = QString::null );

	void redrawScreen();

protected:
	void	gotoxy(int nextx, int nexty);
	
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

	QValueList<yz_event> events;

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

