/**
 * yz_interface.c
 *
 */


#include <stdlib.h>
#include <curses.h>
#include "yz_view.h"

/*
 * C++ api
 */

YZView::YZView(YZBuffer *_b, int _lines_vis)
{
	buffer		= _b;
	lines_vis	= _lines_vis;
	current		= 0;
	cursor_x = cursor_y = cursor_x_ghost = 0;
	mode 		= YZ_VIEW_MODE_COMMAND;
	current_maxx = buffer->find_line(cursor_y)->len-1;

	events_nb_begin = 0;
	events_nb_last = 0;
	

	buffer->add_view(this);
}


/* Used by the buffer to post events */
void YZView::send_char( unicode_char_t c)
{
	if ('\033'==c) {
		mode = YZ_VIEW_MODE_COMMAND;
		post_event(mk_event_setstatus("Command mode"));
		return;
	}
	switch(mode) {
		case YZ_VIEW_MODE_INSERT:
			/* handle adding a char */
			buffer->add_char(cursor_x,cursor_y,c);
			cursor_x++;
			return;
		case YZ_VIEW_MODE_REPLACE:
			/* handle replacing a char */
			buffer->chg_char(cursor_x,cursor_y,c);
			cursor_x++;
			return;
		case YZ_VIEW_MODE_COMMAND:
			/* will be handled after the switch */
			break;
		default:
			/* ?? */
			error("unknown mode, ignoring");
			return;

	};
	/* ok, here we now we're in command */
	switch (c) {
		default:
			post_event(mk_event_setstatus("*Unknown command*"));
			break;
		case 'A': /* append -> insert mode */
			/* go to end of line */
		/* pass through */
		case 'a': /* append -> insert mode */
			mode = YZ_VIEW_MODE_INSERT;
			post_event(mk_event_setstatus("-- INSERT --"));
			break;
		case 'R': /* -> replace mode */
			mode = YZ_VIEW_MODE_REPLACE;
			post_event(mk_event_setstatus("-- REPLACE --"));
			break;
		case 'j': /* move down */
			if (cursor_y<buffer->lines_nb-1) {
				current_maxx = buffer->find_line(++cursor_y)->len-1;
				cursor_x = cursor_x_ghost;
				if (cursor_x>current_maxx) cursor_x = current_maxx;
				if (cursor_x<0) cursor_x = 0;
				update_cursor();
			}
			break;
		case 'k': /* move up */
			if (cursor_y>0) {
				current_maxx = buffer->find_line(--cursor_y)->len-1;
				cursor_x = cursor_x_ghost;
				if (cursor_x>current_maxx) cursor_x = current_maxx;
				if (cursor_x<0) cursor_x = 0;
				update_cursor();
			}
			break;
		case 'h': /* move left */
			if (cursor_x>0) {
				cursor_x_ghost = --cursor_x;
				update_cursor();
			}
			break;
		case 'l': /* move right */
			if (cursor_x<current_maxx) {
				cursor_x_ghost = ++cursor_x;
				update_cursor();
			}
			break;

	}
}


void YZView::update_cursor(void)
{
	post_event( mk_event_setcursor(cursor_x,cursor_y-current));
	debug("posting event about moving cursor to %d,%d, maxx is %d", cursor_x, cursor_y-current,current_maxx);
}

yz_event *YZView::fetch_event(/* asasdfasf */)
{
//	debug("fetch_event");
	if (events_nb_last==events_nb_begin)
		return NULL;

	yz_event *e = &events[events_nb_begin];

	events_nb_begin++;
	if (events_nb_begin>=YZ_EVENT_EVENTS_MAX)
		events_nb_last=0;

	return e;
}

void YZView::post_event (yz_event e)
{
//	debug("post_event");
	events[events_nb_last++] = e;
	if (events_nb_last>=YZ_EVENT_EVENTS_MAX)
		events_nb_last=0;
	if (events_nb_last==events_nb_begin)
		panic("YZ_EVENT_EVENTS_MAX exceeded");
}


/*
 * C api
 */

/*
 * constructors
 */
yz_view create_view(yz_buffer b, int lines_vis)
{
	CHECK_BUFFER(b);
	return (yz_view)(new YZView(buffer, lines_vis));
}



void yz_view_send_char(yz_view v, unicode_char_t c)
{
	CHECK_VIEW(v);
	view->send_char(c);
}

yz_event * yz_view_fetch_event(yz_view v)
{
	CHECK_VIEW(v);
	return view->fetch_event();
}

void yz_view_get_geometry(yz_view v, int *current, int *lines_vis)
{
	CHECK_VIEW(v);
	*current	= view->get_current();
	*lines_vis		= view->get_lines_visible();
}

void yz_view_post_event (yz_view v, yz_event e )
{
	CHECK_VIEW(v);
	return view->post_event(e);
}



