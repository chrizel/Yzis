/*
 * MicroBuffer implementation
 */


#include <unistd.h>
#include <string.h>
#include "yz_microbuffer.h"


MicroBuffer::MicroBuffer(int size)
{
	len = 0;
	len_max = size;
//	data = new char[size];
	data = (char*)malloc(sizeof(char)*size);
	if (!data) panic("memory allocation problem");
	p_next = NULL;
	line = -1;
}


MicroBuffer::MicroBuffer(int _line, char *init, int initlen, int size)
{
	::MicroBuffer(size+initlen);
	append(init, initlen);
	line = _line;
}


MicroBuffer::~MicroBuffer()
{
//	delete[] data;
	free(data);
}


void MicroBuffer::expand(int increase)
{
	if (increase<=0) {
		warning("called with increase<=0, strange, ignored");
		return;
	}
	len_max += increase;
	data = (char*)realloc(data, sizeof(char)*len_max);
	if (!data) panic("memory allocation problem");
}

void MicroBuffer::write_to(int fd)
{
	if (len>0)
		write(fd, data, len);
}

void MicroBuffer::append(char *_data, int _len)
{
	/* checks */
	if (_len<0) panic("called with negative _len : %d", _len);
	if (len+_len>len_max)
		expand(len+_len+YZ_MICROBUFFER_DEFAULT_SIZE);
	/* actually append */
	memcpy(data+len, _data, _len);
}



