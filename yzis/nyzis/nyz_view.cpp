/**
 * nyz_view.cpp
 */

#include "nyzis.h"


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
	YZLine	*yzl;

	if (!e) panic("oulalal c'est pas bon la..");

	switch(e->id) {
		case YZ_EV_SETLINE:
			l = e->u.setline.y;
			if (l<0 or l>=YZ_MAX_LINE) panic("Can't handle more than 3000 lines yet, kinda sucks, uh?");
			yzl = local_lines[l] = e->u.setline.line;

			/* not use addnstr here, will stop at \0  (i guess) */
			for (i=0; i<yzl->len; i++)
				addch(yzl->data[i]);

//			refresh();
//			debug("YZ_EV_SETLINE: received, line is %d", l);
			break;
		case YZ_EV_SETCURSOR:
			move( e->u.setcursor.x, e->u.setcursor.y) ;
//			debug("YZ_EV_SETCURSOR: received");
			break;
		case YZ_EV_SETSTATUS:
			{
				int y,x;
				getyx(stdscr, y, x);
				mvaddstr(LINES-1,0, e->u.setstatus.text);
				move(y,x);
			}
			break;
		default:
			warning("Unhandled event from yzis core : %i", e->id);
			break;
	}
}


