/**
 *
 */

#include "nyzis.h"


void NYZView::event_loop()
{
	/* event loop */
	for (;;) {
		yz_event * e;

		/* got one char ? */
		/* this is a _basic_ event loop... will be improved */
		send_char( getch() );

		/* flush event list */
		while ( (e=fetch_event()) != NULL ) {
			handle_event(e);
		}

	}
}
	

void NYZView::handle_event(yz_event *e)
{
	if (!e) panic("oulalal c'est pas bon la..");
	switch(e->id) {
		case YZ_EV_SETLINE:
			break;
		case YZ_EV_SETCURSOR:
			break;
		default:
			warning("Unhandled event from yzis core : %i", e->id);
			break;
	}
}


