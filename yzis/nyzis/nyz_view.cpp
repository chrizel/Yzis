/**
 * nyz_view.cpp
 */

#include "nyz_view.h"
#include "nyz_session.h"



NYZView::NYZView(NYZSession *_session, WINDOW *_window, YZBuffer *b, int lines_vis)
	: YZView(b,lines_vis)
{
	session= _session;
	window = _window;

	update_info();
	//debug("w,h are %d,%d",w,h);
}

void NYZView::postEvent( yz_event ev ) {

}

void NYZView::event_loop()
{
	/* event loop */
	for (;;) {
		int c;
		/* this is a _basic_ event loop... will be improved */
		c = getch();
		if (c!=ERR)
			send_char( c );
		flush_events();
	}
}
	

void NYZView::flush_events(void)
{
	yz_event * e;

	/* flush event list */
	while ( (e=fetch_event()) != NULL )
		handle_event(e);
}

void NYZView::handle_event(yz_event *e)
{
	int l, i;
	QString	yzl;

//FIXME	if (!e) panic("oulalal c'est pas bon la..");

	switch(e->id) {
		case YZ_EV_SETLINE:
			l = e->u.setline.y;
//FIXME			if (l<0 or l>=YZ_MAX_LINE) panic("Can't handle more than 3000 lines
			//yet, kinda sucks, uh?");
			yzl = mText[l] = *( e->u.setline.line );
			mText.insert( l,*( e->u.setline.line ));

			/* not use addnstr here, will stop at \0  (i guess) */
			move ( l, 0);
			for (i=0; i<w && i<yzl.length(); i++)
				addch(yzl[i].unicode());

//			refresh();
//			debug("YZ_EV_SETLINE: received, line is %d", l);
			break;
		case YZ_EV_SETCURSOR:
			move( e->u.setcursor.y, e->u.setcursor.x) ;
//			debug("YZ_EV_SETCURSOR: received");
			break;
		case YZ_EV_SETSTATUS:
			session->update_status( e->u.setstatus.text );
			break;
		default:
			//FIXME warning("Unhandled event from yzis core : %i", e->id);
			break;
	}
}


