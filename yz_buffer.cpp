/**
 * YZBuffer implementation
 */


#include <stdlib.h>
#include "yz_buffer.h"


/*
 * C++ api
 */

YZBuffer::YZBuffer(char *_path)
{
	path = _path;
}

void YZBuffer::addchar (int x, int y, unicode_char_t c)
{
}


void YZBuffer::chgchar (int x, int y, unicode_char_t c)
{
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


