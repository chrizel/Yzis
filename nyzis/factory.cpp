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

#include <unistd.h>
#include "debug.h"
#include <ctype.h>

#include "factory.h"

NYZFactory *NYZFactory::self = 0;
NYZView *NYZFactory::currentView=0;


NYZFactory::NYZFactory( int argc, char **charv, const char *session_name)
	:YZSession( session_name )
{

	/* init screen */

	(void) initscr();	/* initialize the curses library */
	keypad(stdscr, TRUE);	/* enable keyboard mapping */
	(void) nonl();		/* tell curses not to do NL->CR/NL on output */
	(void) cbreak();	/* take input chars one at a time, no wait for \n */
	(void) noecho();	/* echo input - in color */
	(void) nodelay(stdscr, TRUE);

	if ( self ) {
		yzError( ) << "Instanciating several NYZFactory, should Never happen, quitting..";
		exit(1);
	}
	self = this;

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

	initialiseKeycodes();

	/*
	 * create a buffer and a view on it
	 */
	NYZisDoc *bf;
	if ( argc>1 )
		bf = createBuffer(charv[1]);
	else {
		char tmpname[ 20 ];
		strcpy( tmpname, "/tmp/nyzis.XXXXXX" );
		int fd;
		fd = mkstemp(tmpname);
		if ( -1 == fd ) {
			yzError() << "nyzis : can't open temporary file, quitting"; 
			exit(1);
		}
		close (fd); // maybe we should have a createBuffer(int fd) ? 
		bf = createBuffer(tmpname);
	}

	screen = stdscr; // just an alias...
	wattron(screen, A_BOLD);	// will be herited by subwin

	infobar = subwin(screen, 1, 0, LINES-2, 0);
	wattrset(infobar, A_STANDOUT || A_BOLD);
	wbkgd(infobar, A_REVERSE);
	statusbar  = subwin(screen, 1, STATUSBARWIDTH, LINES-1, 0);
	commandbar = subwin(screen, 1, 0, LINES-1, STATUSBARWIDTH);
	//	(void) notimeout(stdscr,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */
	//	(void) notimeout(window,TRUE);/* prevents the delay between hitting <escape> and when we actually receive the event */

	if (has_colors()) {
		//		wattron(infobar, COLOR_PAIR(4));
		wattron(statusbar, COLOR_PAIR(6));
		wattron(commandbar, COLOR_PAIR(4));
	}

	createView(bf);
}

NYZFactory::~NYZFactory( )
{
	self = 0;
}

void NYZFactory::event_loop() {
	if ( !currentView )
		yzError() << "NYZFactory::event_loop : arghhhhhhh event_loop called with no currentView";
	/* main and only event loop in nyzis */
	for (;;) {
		/* this is a _basic_ event loop... will be improved */
		int c = getch();
		if (c!=ERR) {
			int modifiers = 0;
			if ( isupper( c ) ) modifiers |= Qt::ShiftButton;
			if ( iscntrl( c ) ) modifiers |= Qt::ControlButton;
			//TODO: ALT/META	
			if ( keycodes.contains ( c ) )
				currentView->sendKey( keycodes[ c ], modifiers );
			else
				currentView->sendKey( c, modifiers );
		} else usleep (400);
	}
}

void NYZFactory::update_infobar(int l, int c1, int c2, const QString& percentage) {
	int h,w;
	char * myfmt;

	getmaxyx(infobar, h, w);
	werase(infobar);

	// prevent  gcc to use string
	if ( c1!=c2 ) {
		myfmt="%d,%d-%d";
		mvwprintw( infobar, 0, w-17, myfmt, l,c1,c2 );
	} else {
		myfmt="%d,%d";
		mvwprintw( infobar, 0, w-17, myfmt, l,c1 );
	}
	mvwprintw( infobar, 0, w-4, myfmt, ( const char* )(percentage.local8Bit()) );

	wrefresh(infobar);
}


void NYZFactory::scrollDown( int /*lines*/ ) {

}

void NYZFactory::scrollUp ( int /*lines*/ ) {

}

void NYZFactory::setStatusText( const QString& text )
{
	werase(statusbar);
	waddstr(statusbar, text.local8Bit());
	wrefresh(statusbar);
}


void NYZFactory::quit( bool /*savePopup*/ ) {
	//FIXME
	exit( 0 );
}

void NYZFactory::changeCurrentView ( YZView * view  )
{
	currentView = static_cast<NYZView*>(view);
	currentViewChanged(view);
}

YZView* NYZFactory::createView( YZBuffer* buffer ) {
	WINDOW *window = subwin(screen, LINES-2, 0, 0, 0);
	currentView = new NYZView(window, buffer);
	currentViewChanged(currentView);
	return currentView;
}

NYZisDoc *NYZFactory::createBuffer(const QString& path) {
	NYZisDoc *b = new NYZisDoc( path );
	return b;
}

void NYZFactory::popupMessage( const QString& /* message */ ) {
	//TODO
}

void NYZFactory::deleteView() {
	delete currentView;
	currentView = static_cast<NYZView*>(nextView());
	if ( !currentView )
		yzError() << "nyzys untested when no view is available...";
	// TODO ; some kind of fake view when no view is available..
}

void NYZFactory::setCommandLine( const QString& text ) 
{
	commandline = text;
	werase(commandbar);
	waddstr(commandbar, text.local8Bit());
	wrefresh(commandbar);
}

QString NYZFactory::getCommandLine() const 
{
	return commandline;
}

void NYZFactory::initialiseKeycodes() {
	//initialise keycodes translation map
	keycodes[ KEY_ESCAPE ] = Qt::Key_Escape;
	keycodes[ KEY_ENTER ] = Qt::Key_Return;
	keycodes[ KEY_RETURN ] = Qt::Key_Return;
	//keycodes[ KEY_CODE_YES ] = ;
	//keycodes[ KEY_MIN ] = ;
	keycodes[ KEY_BREAK ] = Qt::Key_Escape;
	//keycodes[ KEY_SRESET ] = ;
	//keycodes[ KEY_RESET ] = ;
	keycodes[ KEY_DOWN ] = Qt::Key_Down;
	keycodes[ KEY_UP ] = Qt::Key_Up;
	keycodes[ KEY_LEFT ] = Qt::Key_Left;
	keycodes[ KEY_RIGHT ] = Qt::Key_Right;
	keycodes[ KEY_HOME ] = Qt::Key_Home;
	keycodes[ KEY_BACKSPACE ] = Qt::Key_Backspace;
	//keycodes[ KEY_F0 ] = Qt::Key_F0;
	keycodes[ KEY_F(1) ] = Qt::Key_F1;
	keycodes[ KEY_F(2) ] = Qt::Key_F2;
	keycodes[ KEY_F( 3 ) ] = Qt::Key_F3;
	keycodes[ KEY_F( 4 ) ] = Qt::Key_F4;
	keycodes[ KEY_F( 5 ) ] = Qt::Key_F5;
	keycodes[ KEY_F( 6 ) ] = Qt::Key_F6;
	keycodes[ KEY_F( 7 ) ] = Qt::Key_F7;
	keycodes[ KEY_F( 8 ) ] = Qt::Key_F8;
	keycodes[ KEY_F( 9 ) ] = Qt::Key_F9;
	keycodes[ KEY_F( 10 ) ] = Qt::Key_F10;
	keycodes[ KEY_F( 11 ) ] = Qt::Key_F11;
	keycodes[ KEY_F( 12 ) ] = Qt::Key_F12;
	//keycodes[ KEY_DL ] = ;
	//keycodes[ KEY_IL ] = ;
	keycodes[ KEY_DC ] = Qt::Key_Delete;
	keycodes[ KEY_IC ] = Qt::Key_Insert;
	//keycodes[ KEY_EIC ] = ;
	keycodes[ KEY_CLEAR ] = Qt::Key_Clear;
	//keycodes[ KEY_EOS ] = ;
	//keycodes[ KEY_EOL ] = ;
	//keycodes[ KEY_SF ] = ;
	//keycodes[ KEY_SR ] = ;
	keycodes[ KEY_NPAGE ] = Qt::Key_Next;
	keycodes[ KEY_PPAGE ] = Qt::Key_Prior;
	//keycodes[ KEY_STAB ] = ;
	//keycodes[ KEY_CTAB ] = ;
	//keycodes[ KEY_CATAB ] = ;
	keycodes[ KEY_ENTER ] = Qt::Key_Return;
	keycodes[ KEY_PRINT ] = Qt::Key_Print;
	//keycodes[ KEY_LL ] = ;
	keycodes[ KEY_A1 ] = Qt::Key_Home;
	keycodes[ KEY_A3 ] = Qt::Key_Prior;
	//keycodes[ KEY_B2 ] = ;
	keycodes[ KEY_C1 ] = Qt::Key_End;
	keycodes[ KEY_C3 ] = Qt::Key_Next;
	keycodes[ KEY_BTAB ] = Qt::Key_Backtab;
	//keycodes[ KEY_BEG ] = ;
	//keycodes[ KEY_CANCEL ] = ;
	//keycodes[ KEY_CLOSE ] = ;
	//keycodes[ KEY_COMMAND ] = ;
	//keycodes[ KEY_COPY ] = ;
	//keycodes[ KEY_CREATE ] = ;
	keycodes[ KEY_END ] = Qt::Key_End;
	//keycodes[ KEY_EXIT ] = ;
	//keycodes[ KEY_FIND ] = ;
	//keycodes[ KEY_HELP ] = ;
	//keycodes[ KEY_MARK ] = ;
	//keycodes[ KEY_MESSAGE ] = ;
	//keycodes[ KEY_MOVE ] = ;
	//keycodes[ KEY_NEXT ] = ;
	//keycodes[ KEY_OPEN ] = ;
	//keycodes[ KEY_OPTIONS ] = ;
	//keycodes[ KEY_PREVIOUS ] = ;
	//keycodes[ KEY_REDO ] = ;
	//keycodes[ KEY_REFERENCE ] = ;
	//keycodes[ KEY_REFRESH ] = ;
	//keycodes[ KEY_REPLACE ] = ;
	//keycodes[ KEY_RESTART ] = ;
	//keycodes[ KEY_RESUME ] = ;
	//keycodes[ KEY_SAVE ] = ;
	//keycodes[ KEY_SBEG ] = ;
	/*keycodes[ KEY_SCANCEL ] = ;
		keycodes[ KEY_SCOMMAND ] = ;
		keycodes[ KEY_SCOPY ] = ;
		keycodes[ KEY_SCREATE ] = ;
		keycodes[ KEY_SDC ] = ;
		keycodes[ KEY_SDL ] = ;
		keycodes[ KEY_SELECT ] = ;
		keycodes[ KEY_SEND ] = ;
		keycodes[ KEY_SEOL ] = ;
		keycodes[ KEY_SEXIT ] = ;
		keycodes[ KEY_SFIND ] = ;
		keycodes[ KEY_SHELP ] = ;
		keycodes[ KEY_SHOME ] = ;
		keycodes[ KEY_SIC ] = ;
		keycodes[ KEY_SLEFT ] = ;
		keycodes[ KEY_SMESSAGE ] = ;
		keycodes[ KEY_SMOVE ] = ;
		keycodes[ KEY_SNEXT ] = ;
		keycodes[ KEY_SOPTIONS ] = ;
		keycodes[ KEY_SPREVIOUS ] = ;
		keycodes[ KEY_SPRINT ] = ;
		keycodes[ KEY_SREDO ] = ;
		keycodes[ KEY_SREPLACE ] = ;
		keycodes[ KEY_SRIGHT ] = ;
		keycodes[ KEY_SRSUME ] = ;
		keycodes[ KEY_SSAVE ] = ;
		keycodes[ KEY_SSUSPEND ] = ;
		keycodes[ KEY_SUNDO ] = ;
		keycodes[ KEY_SUSPEND ] = ;
		keycodes[ KEY_UNDO ] = ;
		keycodes[ KEY_MOUSE ] = ;
		keycodes[ KEY_RESIZE ] = ;
		keycodes[ KEY_EVENT ] = ;
		keycodes[ KEY_MAX ] = ;*/
}



