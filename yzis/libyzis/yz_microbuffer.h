#ifndef MICROBUFFER_H
#define MICROBUFFER_H


#include "yz_line.h"


/**
  *
  *
  */

#define YZ_MICROBUFFER_DEFAULT_SIZE	4096

#ifdef __cplusplus
class MicroBuffer {

public:
	MicroBuffer(int size=YZ_MICROBUFFER_DEFAULT_SIZE);
	/**
	  * @param line : line who corresponds in the file
	  * @param init : the data to be stored in this MicroBuffer
	  * @param datalen : length of data to be taken from @param init
	  * @param size : initial size of this MicroBuffer, the actual size will
	  * be at least max(@param size, @param datalen)
	  */
	MicroBuffer(int _line, char *init, int datalen, int size=YZ_MICROBUFFER_DEFAULT_SIZE);

	~MicroBuffer();

	void fill(YZLine &);
	void write_to(int fd);
	void append(char *data, int len);
	MicroBuffer * next(void) { return p_next; }
	void	set_next(MicroBuffer *n) { p_next = n; }

	int	line;
protected:
	MicroBuffer * p_next;
	void	expand(int increase); 

	char	*data;
	int	len;
	int	len_max;
};
#endif // __cplusplus

#endif // MICROBUFFER_H
