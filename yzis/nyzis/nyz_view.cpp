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
	switch(e->id) {
		case YZ_EV_SETLINE:
			debug("YZ_EV_SETLINE: received, line is %d", e->u.setline.y);
			break;
		case YZ_EV_SETCURSOR:
			debug("YZ_EV_SETCURSOR: received");
			break;
		default:
			warning("Unhandled event from yzis core : %i", e->id);
			break;
	}
}


