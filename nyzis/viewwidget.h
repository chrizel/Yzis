/**
 * $Id$
 */
#ifndef NYZ_VIEW_H
#define NYZ_VIEW_H
/**
 * ncurses-based GUI for yzis
 */

#include "view.h"
#include <curses.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qnamespace.h>

#define KEY_ESCAPE 27
#ifdef KEY_ENTER
#undef KEY_ENTER
#define KEY_ENTER 10
#endif
#define KEY_RETURN 13

class NYZSession;

class NYZView : public YZView {
public:
	/**
	  * constructor. Each view is binded to a buffer, @arg lines is the initial number of lines that
	  * this view can display
	  */
	NYZView(NYZSession *_session, WINDOW *_window, YZBuffer *b);
	virtual ~NYZView();

	void event_loop();
	void flush_events();

protected:
	void handle_event(yz_event e);

	WINDOW		*window;	/* ncurses window to write to */
	NYZSession	*session;

	unsigned int	h, w;		/** height and width of the window */

private:
	void update_info(void) { getmaxyx(window, h, w); mLinesVis = h; }
	/**
	  * Display a line
	  * @arg line is the line number, taken from the beginning of the file ( and not
	  * the beginning of the displayed part )
	  */
	void printLine( int line );
	/**
	  * print a void line ( that is "~    " )
	  * @arg line is the line # relative to the displayed screen, not the buffer ( as this
	  * doesn't belong to the buffer anyway..)
	  */
	void printVoid( unsigned int line );

	/**
	 * Fill the map of keycodes -> Ncurses to Qt
	 */
	void initialiseKeycodes();

	//XXX static ?
	QMap<int,Qt::Key> keycodes; // map Ncurses to Qt codes

};

#endif // NYZ_VIEW_H
