/**
 * YZBuffer implementation
 */


#include <stdlib.h>
#include "yz_buffer.h"
#include "yz_events.h"
#include "yz_view.h"


/*
 * C++ api
 */

YZBuffer::YZBuffer(char *_path)
{
	if (_path!=NULL) panic("non-null path not yet implemented");
	path	= _path;
	view_nb	= 0;

	mb_first = mb_last = NULL; // linked mb
}

void YZBuffer::addchar (int x, int y, unicode_char_t c)
{
	yz_event e;
	/* do the actual modification */

	post_event(e); /* inform the views */
}


void YZBuffer::chgchar (int x, int y, unicode_char_t c)
{
	yz_event e;
	/* do the actual modification */

	post_event(e); /* inform the views */
}

void YZBuffer::post_event(yz_event e)
{
	int l = e.u.setline.y;

	/* quite basic, we just check that the line is currently displayed */
	for (int i=0; i<view_nb; i++) {
		YZView *v = view_list[i];
		if (v->is_line_visible(l))
			view_list[i]->post_event(e);
	}
}

void YZBuffer::addview (YZView *v)
{
	if (view_nb>YZ_MAX_VIEW)
		panic("no more rom for new view in YZBuffer");

	//debug("adding view %p, number is %d", v, view_nb);
	view_list[view_nb++] = v;
} 


void YZBuffer::load(void)
{
	if (!path) {
		error("called though path is null, ignored");
		return;
	}
}

void YZBuffer::save(void)
{
	if (!path) {
		error("called though path is null, ignored");
		return;
	}
}


/*
 * C api
 */

/*
 * constructors
 */
yz_buffer create_empty_buffer(void) { return (yz_buffer)(new YZBuffer()); }
yz_buffer create_buffer(char *_path) { return (yz_buffer)(new YZBuffer(_path)); }


/*
 * functions
 */
void buffer_addchar(yz_buffer b , int x, int y, unicode_char_t c)
{
	CHECK_BUFFER(b);
	buffer->addchar(x,y,c);
}


void buffer_chgchar(yz_buffer b, int x, int y, unicode_char_t c)
{
	CHECK_BUFFER(b);
	buffer->chgchar(x,y,c);
}


void buffer_load(yz_buffer b)
{
	CHECK_BUFFER(b);
	buffer->load();
}

void buffer_save(yz_buffer b)
{
	CHECK_BUFFER(b);
	buffer->save();
}


