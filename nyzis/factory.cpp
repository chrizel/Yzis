/*
	  Copyright (c) 2003 Yzis Team <yzis-dev@yzis.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/**
 * $Id$
 */

#include "factory.h"

NYZSession::NYZSession( int argc, char **charv, const char *_session_name)
:YZSession(_session_name) {

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
	YZBuffer *bf;
	if ( argc>1 )
		bf = createBuffer(charv[1]);
	else
		//WRONG XXX  has to be a real file (mm)
		bf = createBuffer("");

	screen = stdscr; // just an alias...
	wattron(screen, A_BOLD);	// will be herited by subwin

	infobar = subwin(screen, 1, 0, LINES-2, 0);
	wattrset(infobar, A_STANDOUT || A_BOLD);
	wbkgd(infobar, A_REVERSE);

	statusbar = subwin(screen, 1, 0, LINES-1, 0);
	WINDOW *window = subwin(screen, LINES-2, 0, 0, 0);

	//	(void) notimeout(stdscr,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */
	//	(void) notimeout(window,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */

	if (has_colors()) {
		//		wattron(infobar, COLOR_PAIR(4));
		wattron(statusbar, COLOR_PAIR(6));
	}

	registerManager( this );
	new NYZView(this, window, bf);
}

void NYZSession::postEvent( yz_event /*ev*/ ) {
	// do nothing, we'll catch them in next flush_events()
}


void NYZSession::event_loop() {
	for ( QMap<QString,YZBuffer*>::Iterator b = mBuffers.begin();b!=mBuffers.end(); ++b ) 
		for ( QValueList<YZView*>::iterator it = b.data()->views().begin() ; it!=b.data()->views().end() ; it++ ) {
			YZView *v = *it;
			( static_cast<NYZView*>( v ) )->event_loop();
		}
}

void NYZSession::update_status(const QString& msg) {
	werase(statusbar);
	waddstr(statusbar, msg.local8Bit());

	wrefresh(statusbar);
}


void NYZSession::update_infobar(int l, int c1, int c2, const QString& percentage) {
	int h,w;
	char * myfmt;

	getmaxyx(infobar, h, w);
	werase(infobar);

	// prevent  gcc to use string
	myfmt="%d,%d-%d";
	mvwprintw( infobar, 0, w-17, myfmt, l,c1,c2 );
	myfmt="%s";
	mvwprintw( infobar, 0, w-4, myfmt, ( const char* )(percentage.local8Bit()) );

	wrefresh(infobar);
}


void NYZSession::scrollDown( int /*lines*/ ) {

}

void NYZSession::scrollUp ( int /*lines*/ ) {

}

void NYZSession::setCommandLineText( const QString& text ) {
	commandline= text;
	update_status( text ); //XXX will that work ?
}

QString NYZSession::getCommandLineText() const {
	return commandline;
}

void NYZSession::quit( bool savePopup ) {
	//FIXME
	exit( 0 );
}

void NYZSession::setCurrentView ( YZView *view ) {
 
}

YZView* NYZSession::createView( YZBuffer* buffer ) {
	//TODO
}

YZBuffer *NYZSession::createBuffer(const QString& path) {
	YZBuffer *b = new YZBuffer( this, path );
	addBuffer( b );
	return b;
}

void NYZSession::popupMessage( const QString& message ) {
	//TODO
}
