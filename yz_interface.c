/**
 * yz_interface.c
 *
 */


#include <stdlib.h>
#include "yz_interface.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifndef NULL
#define NULL 0
#endif


/*
 * yz_buffer
 */

yz_buffer *create_empty_buffer(void)
{
	return create_buffer(NULL);
}


yz_buffer *create_buffer(char *path)
{
	yz_buffer * b = malloc(sizeof(yz_buffer));
	b->path = path;
	return b;
}


void buffer_addchar(yz_buffer *, int x, int y, )
{
}


void buffer_chgchar(yz_buffer *, int x, int y, )
{
}


/*
 * yz_view
 */

yz_view *create_view(yz_buffer *b, int lines)
{
	yz_view * v = malloc(sizeof(yz_view));
	v->buffer	= b;
	v->lines	= lines;
	v->current	= 0;
	v->x		= 0;
	v->y		= 0;
	v->mode		= YZ_VIEW_MODE_COMMAND;

	return v;
}

void yz_send_char(yz_view *v, char a)
{
	/* nothing yet */
}

yz_event * yz_fetch_event(yz_view *v)
{
	/* nothing yet */
	yz_event *e = NULL;

	return e;
}

void yz_get_geometry(yz_view *v, int *current, int *lines)
{
	*current	= v->current;
	*lines		= v->lines;
}


#ifdef __cplusplus

/*
 * C++ API
 */



void	YZView::fetch_event(void)
{
}


/* End of c++ API */
#endif


