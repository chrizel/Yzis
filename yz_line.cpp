/*
 * YZLine implementation
 */


#include <unistd.h>
#include <string.h>
#include "yz_line.h"


YZLine::YZLine(int size)
{
	len = 0;
	flags = 0;
	len_max = size;
//	data = new char[size];
	data = (char*)malloc(sizeof(char)*size);
	if (!data) panic("memory allocation problem");
	p_next = NULL;
	line = -1;
}


YZLine::YZLine(int _line, char *init, int initlen, int size)
{
	::YZLine(size+initlen);
	append(init, initlen);
	line = _line;
}


YZLine::~YZLine()
{
//	delete[] data;
	free(data);
}


void YZLine::expand(int newsize)
{
	if (newsize<=len_max) {
		warning("called with newsize<=len_max, strange, ignored");
		return;
	}
	debug("expanding upto %d", newsize);
	len_max = newsize;
	data = (char*)realloc(data, sizeof(char)*len_max);
	if (!data) panic("memory allocation problem");
}

void YZLine::write_to(int fd)
{
	if (len>0)
		write(fd, data, len);
}

void YZLine::append(char *_data, int _len)
{
	/* checks */
	if (_len<0) panic("called with negative _len : %d", _len);
	if (len+_len>len_max)
		expand(len+_len+YZ_LINE_DEFAULT_LENGTH);
	/* actually append */
	memcpy(data+len, _data, _len);
}

void YZLine::add_char (int x, unicode_char_t c)
{
	/* not optimised */
	if (len>=len_max) expand(len_max+YZ_LINE_DEFAULT_LENGTH);
	int i;
	for (i=len-1; i>x; i--)
		data[i]=data[i-1];
	data[x] = c;
}


void YZLine::chg_char (int x, unicode_char_t c)
{
	/* not optimised */
	data[x] = c;
}





