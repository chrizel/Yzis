/**
 * yz_interface.h
 *
 * YZBuffer - abstractino for buffer/file
 */


#include "yz_line.h"

#ifdef __cplusplus

class YZView;


class YZBuffer {

	friend class YZView;

public:
	/** opens a buffer using the given file */
	YZBuffer(char *_path=NULL);

	void addchar (int x, int y, unicode_char_t c);
	void chgchar (int x, int y, unicode_char_t c);
protected:
	char * path;

	/* readonly?, change, load, save, isclean?, ... */
	/* locking stuff will be here, too */
};

#endif



/*
 * C API
 */

#ifdef __cplusplus
extern "C" {
#endif


typedef int *yz_buffer;
#if 0
#define CHECK_BUFFER(yz_buffer) YZBuffer *buffer; if (!yz_buffer || (buffer=(YZBuffer*)(yz_buffer))->magic != MAGIC_YZBUFFER) panic("");
#else
#define CHECK_BUFFER(yz_buffer) YZBuffer *buffer=(YZBuffer*)(yz_buffer);
#endif


/** creates an empty buffer */
yz_buffer create_empty_buffer(void);
/** opens a buffer using the given file */
yz_buffer create_buffer(char *path);

/** add a character on the (x,y) position, move all the line to right */
void buffer_addchar(yz_buffer , int x, int y, unicode_char_t c);
/** change the character on the (x,y) position, hence erasing the current one */
void buffer_chgchar(yz_buffer , int x, int y, unicode_char_t c);


#ifdef __cplusplus
}
#endif
