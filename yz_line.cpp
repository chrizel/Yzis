/*
 * YZLine implementation
 */


#include <unistd.h>
//#include <string.h>
#include "yz_line.h"


/*YZLine::YZLine(int size) 
	: QString(size)
{
	len = 0;
	flags = 0;
	len_max = size;
	p_next = NULL;
	line = -1;
}*/

YZLine::YZLine(int _line, QString init)
	: QString(init)
{
	line = _line;
}

YZLine::~YZLine()
{
}

/*void YZLine::write_to(int fd)
{
	if (len>0)
		write(fd, data, len);
}*/

/*void YZLine::append(QString _data, int applen)
{
	if (applen<0) panic("called with negative applen : %d", applen);
	if (len+applen>len_max)
		expand(len+applen+YZ_LINE_DEFAULT_LENGTH);
	memcpy(data+len, _data, applen);
	len+=applen;
}*/

/*
void YZLine::add_char (int x, QChar c)
{
	if (len>=len_max) expand(len_max+YZ_LINE_DEFAULT_LENGTH);
	len++;
	for (int i=len-1; i>x; i--)
		data[i]=data[i-1];
	data[x] = c;
}*/

/*
void YZLine::chg_char (int x, QChar c)
{
	data[x] = c;
}*/





