/**
 * $Id$
 */

#include <qnamespace.h>
#include <ctype.h>
#include "viewwidget.h"
#include "factory.h"

NYZView::NYZView(NYZSession *_session, WINDOW *_window, YZBuffer *b)
	: YZView(b,_session,0)
{
	session= _session;
	window = _window;
	session->commandline="";

	update_info();
	//debug("w,h are %d,%d",w,h);
	//
	wmove(window,0,0 );
	//active symbols for special keycodes
	keypad(window , true);
	//registerManager( this );
}

NYZView::~NYZView(){
}

void NYZView::initialiseKeycodes() {
	//initialise keycodes translation map
	keycodes[ KEY_ESCAPE ] = Qt::Key_Escape;
	keycodes[ KEY_ENTER ] = Qt::Key_Return;
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

void NYZView::event_loop()
{
	/* event loop */
	for (;;) {
		int c;
		/* this is a _basic_ event loop... will be improved */
		c = getch();
		if (c!=ERR) {
			int modifiers = 0;
			if ( isupper( c ) ) modifiers |= YZIS::Shift;
			if ( iscntrl( c ) ) modifiers |= YZIS::Ctrl;
		//TODO: ALT/META	
			if ( keycodes.contains ( c ) )
				sendKey( keycodes[ c ], modifiers );
			else
				sendKey( c, modifiers );
		}
		flush_events();
	}
}

void NYZView::flush_events(void)
{
	yz_event e;

	/* flush event list */
	while ( (e=session->fetchNextEvent(/*myId*/)).id != YZ_EV_NOOP )
		handle_event(e);
}

void NYZView::handle_event(yz_event e)
{

	switch(e.id) {
		case YZ_EV_INVALIDATE_LINE:
			printLine( e.invalidateline.y );

			wrefresh( window );
//			debug("YZ_EV_SET_LINE: received, line is %d", l);
			break;
		case YZ_EV_SET_CURSOR:
			session->update_infobar(
				e.setcursor.x,
				e.setcursor.y,
				e.setcursor.y2,
				e.setcursor.percentage
				);
			wmove(window, e.setcursor.y2 - getCurrent(), e.setcursor.x) ;
			wrefresh( window );
//			debug("YZ_EV_SET_CURSOR: received");
			break;
		case YZ_EV_SET_STATUS:
			session->update_status( e.setstatus.text );
			break;
		case YZ_EV_REDRAW: {
			unsigned int i;
			for ( i=getCurrent(); i<getCurrent()+lines_vis && i<buffer->getLines(); i++ ) {
				printLine(i);
			}
			i-=getCurrent();
			for ( ; i<lines_vis; i++ ) printVoid(i );
		}
			break;
		default:
			//FIXME warning("Unhandled event from yzis core : %i", e->id);
			break;
	}
}

void NYZView::printVoid( int relline ) {

	unsigned int i;

	// clipping
	if ( relline<0 || relline > lines_vis ) return;
	wmove (window, relline, 0);
	waddch(window, '~');
	for (i=1 ; i< w; i++ ) waddch(window, ' ' );
}

void NYZView::printLine( int line ) {

	unsigned int i;
	int sx,sy; // save x,y
	int relline = line - getCurrent(); // relative line #

	// check
	QString str = buffer->findLine( line );
	if ( str.isNull() ) return;

	// clipping 
	if ( relline<0 || relline > lines_vis ) return;

	getyx(window,sy,sx); // save cursor

	/* not use addnstr here, will stop at \0  (i guess) */
	wmove (window, relline, 0);
	for (i=0; i<w && i<str.length(); i++)
		waddch(window, str[i].unicode());
//		addch(str[i].unicode());
	for ( ; i< w; i++ ) waddch(window, ' ' );
	wmove(window,sy,sx ); // restore cursor
}



