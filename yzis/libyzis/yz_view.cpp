/**
 * yz_interface.c
 *
 */


#include <stdlib.h>
#include "yz_view.h"

/*
 * C++ api
 */

void YZView::send_char( unicode_char_t)
{
	// nothing yet;
}

yz_event *YZView::fetch_event(/* asasdfasf */)
{
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



void yz_send_char(yz_view v, unicode_char_t c)
{
	CHECK_VIEW(v);
	view->send_char(c);
}

yz_event * yz_fetch_event(yz_view v)
{
	CHECK_VIEW(v);
	return view->fetch_event();
}

void yz_get_geometry(yz_view v, int *current, int *lines)
{
	CHECK_VIEW(v);
	*current	= view->get_current();
	*lines		= view->get_lines_displayed();
}


