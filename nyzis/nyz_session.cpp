/**
 * $Id: nyz_session.cpp,v 1.9 2003/04/25 12:45:33 mikmak Exp $
 */

#include "nyz_session.h"

NYZSession::NYZSession( int argc, char **charv, const char *_session_name)
	:YZSession(_session_name)
{

	/* init screen */

	(void) initscr();	/* initialize the curses library */
	keypad(stdscr, TRUE);	/* enable keyboard mapping */
	(void) nonl();		/* tell curses not to do NL->CR/NL on output */
	(void) cbreak();	/* take input chars one at a time, no wait for \n */
	(void) noecho();	/* echo input - in color */
	(void) nodelay(stdscr, TRUE);

	if (has_colors()) {
		start_color();

		/*
		 * Simple color assignment, often all we need.  Color pair 0 cannot
		 * be redefined.  This example uses the same value for the color
		 * pair as for the foreground color, though of course that is not
		 * necessary:
		 */
		init_pair(1, COLOR_RED,     COLOR_BLACK);
		init_pair(2, COLOR_GREEN,   COLOR_BLACK);
		init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
		init_pair(4, COLOR_BLUE,    COLOR_BLACK);
		init_pair(5, COLOR_CYAN,    COLOR_BLACK);
		init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(7, COLOR_WHITE,   COLOR_BLACK);
	}

	/*
	 * create a buffer and a view on it
	 */
	YZBuffer *bf = createBuffer(charv[1]);

	screen = stdscr; // just an alias...
		wattron(screen, A_BOLD);	// will be herited by subwin

	infobar = subwin(screen, 1, 0, LINES-2, 0);
		wattron(infobar, A_REVERSE);
		wbkgd(infobar, A_REVERSE);

	statusbar = subwin(screen, 1, 0, LINES-1, 0);
	WINDOW *window = subwin(screen, LINES-2, 0, 0, 0);

//	(void) notimeout(stdscr,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */
//	(void) notimeout(window,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */

	if (has_colors()) {
		wattron(infobar, COLOR_PAIR(4));
		wattron(statusbar, COLOR_PAIR(6));
	}

	bf->addView (new NYZView(this, window, bf, LINES));
}

void NYZSession::event_loop()
{
	for ( QMap<QString,YZBuffer*>::Iterator b = buffers.begin();b!=buffers.end(); ++b ) 
		for ( YZView *v = b.data()->views().first() ; v ; v = b.data()->views().next() ) 
			( static_cast<NYZView*>( v ) )->event_loop();
}

void NYZSession::update_status(QString *msg)
{
	save_cursor();

	werase(statusbar);
	waddstr(statusbar, msg->latin1());
	wrefresh(statusbar);

	restore_cursor();
}

void NYZSession::save_cursor(void)
{
	getyx(screen,save_cursor_y,save_cursor_x);
}

void NYZSession::restore_cursor(void)
{
	move(save_cursor_y,save_cursor_x);
}




