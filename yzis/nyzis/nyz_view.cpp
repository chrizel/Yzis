/**
 * nyz_view.cpp
 */

#include "nyzis.h"


void NYZView::event_loop()
{
	/* event loop */
	for (;;) {
		/* this is a _basic_ event loop... will be improved */
		flush_events();
		send_char( getch() );

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
	if (!e) panic("oulalal c'est pas bon la..");
	int l;
	switch(e->id) {
		case YZ_EV_SETLINE:
			l = e->u.setline.y;
			if (l<0 or l>=YZ_MAX_LINE) panic("Can't handle more than 3000 lines yet, kinda sucks, uh?");
			lines[l] = e->u.setline.line;
			debug("YZ_EV_SETLINE: received, line is %d", l);
			break;
		case YZ_EV_SETCURSOR:
			debug("YZ_EV_SETCURSOR: received");
			break;
		default:
			warning("Unhandled event from yzis core : %i", e->id);
			break;
	}
}


