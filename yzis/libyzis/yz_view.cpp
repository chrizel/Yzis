/**
 * yz_interface.c
 *
 */


#include <stdlib.h>
#include "yz_view.h"

/*
 * C++ api
 */

YZView::YZView(YZBuffer *_b, int _lines)
{
	buffer	= _b;
	lines	= _lines;
	current	= x = y = 0;
	mode 	= YZ_VIEW_MODE_COMMAND;

	events_nb_begin = 0;
	events_nb_last = 0;
	

	buffer->addview(this);
}


void YZView::send_char( unicode_char_t)
{
	// nothing yet;
}

yz_event *YZView::fetch_event(/* asasdfasf */)
{
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
yz_view create_view(yz_buffer b, int lines)
{
	CHECK_BUFFER(b);
	return (yz_view)(new YZView(buffer, lines));
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

void yz_view_get_geometry(yz_view v, int *current, int *lines)
{
	CHECK_VIEW(v);
	*current	= view->get_current();
	*lines		= view->get_lines_displayed();
}

void yz_view_post_event (yz_view v, yz_event e )
{
	CHECK_VIEW(v);
	return view->post_event(e);
}



