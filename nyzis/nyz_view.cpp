/**
 * $Id$
 */

#include "nyz_view.h"
#include "nyz_session.h"

NYZView::NYZView(NYZSession *_session, WINDOW *_window, YZBuffer *b)
	: YZView(b,0)
{
	session= _session;
	window = _window;
	commandline="";

	update_info();
	//debug("w,h are %d,%d",w,h);
	//
	wmove(window,0,0 );
	registerManager( this );
}

NYZView::~NYZView(){
}

void NYZView::postEvent( yz_event /*ev*/ ) {
	// do nothing, we'll catch them in next flush_events()
}

void NYZView::event_loop()
{
	/* event loop */
	for (;;) {
		int c;
		/* this is a _basic_ event loop... will be improved */
		c = getch();
		if (c!=ERR)
			sendChar( c );
		flush_events();
	}
}
	

void NYZView::flush_events(void)
{
	yz_event e;

	/* flush event list */
	while ( (e=fetchNextEvent()).id != YZ_EV_NOOP )
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



void NYZView::scrollDown( int /*lines*/ ) {

}

void NYZView::scrollUp ( int /*lines*/ ) {

}

YZSession *NYZView::getCurrentSession(void)
{
	return session;
}

void NYZView::setCommandLineText( const QString& text ) {
	commandline= text;
}

QString NYZView::getCommandLineText() const {
	return commandline;
}

