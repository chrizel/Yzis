/*
	  Copyright (c) 2003, 2004 Thomas Capricelli <orzel@freehackers.org>

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


NYZFactory::NYZFactory(const char *session_name)
	:YZSession( session_name )
{
	/* init screen */

	(void) initscr();	/* initialize the curses library */
	keypad(stdscr, TRUE);	/* enable keyboard mapping */
	(void) nonl();		/* tell curses not to do NL->CR/NL on output */
	(void) cbreak();	/* take input chars one at a time, no wait for \n */
	(void) noecho();	/* echo input - in color */
	(void) nodelay(stdscr, TRUE);
	(void) intrflush( stdscr, FALSE );

	screen = stdscr; // just an alias...
	wattron(screen, A_STANDOUT);	// will be herited by subwin

	if ( has_colors() ) start_color();

	if ( self ) {
		yzFatal(NYZIS) << "Instanciating several NYZFactory, should Never happen, quitting..";
		exit(1);
	}
	self = this;

	initialiseKeycodes();
}

NYZFactory::~NYZFactory( )
{
	self = 0;
}

void NYZFactory::event_loop()
{
	YZASSERT_MSG( currentView,
			"NYZFactory::event_loop : arghhhhhhh event_loop called with no currentView" );
	/* main and only event loop in nyzis */
	for (;;) {
		/* this is a _basic_ event loop... will be improved */
		int c = getch();
	
		// we know what this is..
		switch( c ){
			case ERR:
				extern uint qGlobalPostedEventsCount();
#if 0
				if ( qApp->hasPendingEvents () ) {
					yzDebug( NYZIS ) << "qt wanna do something.. " << qGlobalPostedEventsCount() << " pending events.." << endl;
					qApp->processEvents( 400 );
				} else usleep( 400 );
#else
				usleep( 400 );
#endif
			// do nothing with the following
			case KEY_RESIZE:
				continue;
			case 15:
				currentView->sendKey('o',Qt::ControlButton );
				continue;
			case 12:
				currentView->sendKey('l',Qt::ControlButton );
				continue;
		}
		if (keycodes.contains(c)) {
			currentView->sendKey(keycodes[c],0 );
			continue;
		}
		// remaining cases
		if ( c& KEY_CODE_YES ) { // ncurses special key
			yzError(NYZIS) << "*************** Unhandled" <<
				"ncurses key code, please report : " << (int) c << endl;
			continue;
			}
		int modifiers = 0;
		if ( c & 0200 ) {
			// heuristic, alt-x is x|0200..
			modifiers |= Qt::AltButton;
			c &= ~0200;
		}
		if ( c>=KEY_MAX) { // non-ascii key
			yzError(NYZIS) << "*************** Unhandled" <<
				"and very strange (>KEY_MAX) char received from ncurses, please report : " << (int) c << endl;
			continue;
			}
		if ( c>127 ) { // non-ascii key
			yzError(NYZIS) << "*************** Unhandled" <<
				"non-ascii key, please report : " << (int) c << endl;
			continue;
			}
		if ( iscntrl( c ) ) {
			yzError(NYZIS) << "*************** Unhandled" <<
				"control sequence " << c << " (discarded)" << endl;
			continue;
		}
		if ( isupper( c ) ) { modifiers |= Qt::ShiftButton; c+= 'a'-'A'; }
		//TODO: META	
		currentView->sendKey( c, modifiers );
	}
}

void NYZFactory::scrollDown( int /*lines*/ ) {

}

void NYZFactory::scrollUp ( int /*lines*/ ) {

}


void NYZFactory::quit( bool /*savePopup*/ ) {
	// TODO
	exit( 0 );
}

void NYZFactory::changeCurrentView ( YZView * view  )
{
	NYZView *v = static_cast<NYZView*>(view);
	YZASSERT( view );
	if ( currentView == v ){
		yzWarning(NYZIS) << "changeCurrentView() called with same view.."<<endl;
		return;
	}
	if ( currentView )
		currentView->unmap();
	currentView = v;
	currentViewChanged(view);
	currentView->map();
}

YZView* NYZFactory::createView( YZBuffer* buffer )
{
	YZASSERT( buffer );
	NYZView *v = new NYZView( buffer );
	YZASSERT_MSG(v, "NYZFactory::createView : failed creating a new NYZView");
	buffer->addView ( v);
	return currentView;
}

NYZisDoc *NYZFactory::createBuffer(const QString& filename)
{
	NYZisDoc *b = new NYZisDoc();
	YZASSERT_MSG(b, "NYZFactory::createBuffer failed creating new NYZisDoc");
	createView( b );
	b->load( filename );
	return b;
}

void NYZFactory::popupMessage( const QString &message )
{
	int nl,nc,y;
#if 0
	message.simplifyWhiteSpace();
	nc = message.length();
	nl = 1;
	if ( nc>COLS-4 && COLS-4>0 ) {
		nl = nc / COLS-4;
		nc = COLS-4;
	}
	WINDOW *popup = newwin(nl+4, nc+4, ( LINES-nl )/2, (COLS-nc)/2);
	box( popup, 0, 0 );

	y=2;
	while (nl ) {
		mvwaddstr( popup, y, 2, message.mid(y*nc, nc).latin1() );
		y++;nl--;
	}
	// TODO : use QString QString::section 
#else
	message.simplifyWhiteSpace();
	nc = message.length();
	nl = 1;
	WINDOW *popup = newwin(nl+4, nc+4, ( LINES-nl )/2, (COLS-nc)/2);
	box( popup, 0, 0 );

	mvwaddstr( popup, 2, 2, message.latin1() );
	// TODO : use QString QString::section 

#endif

	wrefresh(popup);
	refresh();
	sleep (2);
	delwin( popup );
	currentView->refreshScreen();
}

void NYZFactory::deleteView()
{
	NYZView *oldview = currentView;

	// find next one
	YZView *v = nextView();
	if ( !v ) v = prevView();
// ideally would be : but we delteView on quitting, hence looping delete/create..
// TODO
//	if ( !v ) v = createView(createBuffer()); 
	if ( !v ) quit( 0 );

	// switch
	setCurrentView(v);

	// delete
//	delete oldview;// wont work because the oldview is the current one used
	oldview->myBuffer()->detach();

	if (mBuffers.isEmpty()) {
		yzWarning(NYZIS)<<"nyzis can't handle not having any view/buffers, quitting" << endl;;
		quit();
	}
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
	keycodes[  127 ] = Qt::Key_Backspace;
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
	keycodes[ KEY_IC ] = Qt::Key_I;
	keycodes[ Qt::Key_Insert ] = Qt::Key_I;
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


