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
	int l;
	unsigned int i;
	QString	yzl;

	int sx,sy; // save x,y

	switch(e.id) {
		case YZ_EV_SETLINE:
			getyx(window,sy,sx);
			l = e.setline.y - getCurrent();
			yzl = e.setline.line;

			/* not use addnstr here, will stop at \0  (i guess) */
			wmove (window, l, 0);
			for (i=0; i<w && i<yzl.length(); i++)
				waddch(window, yzl[i].unicode());
//				addch(yzl[i].unicode());
			for ( ; i< w; i++ ) waddch(window, ' ' );

			wmove(window,sy,sx );

			wrefresh( window );
//			debug("YZ_EV_SETLINE: received, line is %d", l);
			break;
		case YZ_EV_SETCURSOR:
			wmove(window, e.setcursor.y - getCurrent(), e.setcursor.x) ;
			wrefresh( window );
//			debug("YZ_EV_SETCURSOR: received");
			break;
		case YZ_EV_SETSTATUS:
			session->update_status( e.setstatus.text );
			break;
		default:
			//FIXME warning("Unhandled event from yzis core : %i", e->id);
			break;
	}
}

void NYZView::scrollDown( int lines ) {

}

void NYZView::scrollUp ( int lines ) {

}

YZSession *NYZView::getCurrentSession() {
	return session;
}
