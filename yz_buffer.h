#ifndef YZ_BUFFER_H
#define YZ_BUFFER_H
/**
 * yz_interface.h
 *
 * YZBuffer - abstractino for buffer/file
 */


#include "yz_line.h"
#include "yz_events.h"
#include "yz_microbuffer.h"

#ifdef __cplusplus


#define	YZ_MAX_VIEW 50



class YZView;

class YZBuffer {

	friend class YZView;

public:
	/** opens a buffer using the given file */
	YZBuffer(char *_path=NULL);

	void addchar (int x, int y, unicode_char_t c);
	void chgchar (int x, int y, unicode_char_t c);

	void load(void);
	void save(void);

	/* linked list handling */
	void add_line(YZLine *l) { l->set_next(NULL); if (line_last) line_last->set_next(l); line_last=l; }

protected:
	void add_view (YZView *v);

	char	*path;
	YZView	*view_list[YZ_MAX_VIEW];	// should be growable 
	int	view_nb;

private:
	void	post_event(yz_event e);
	void	update_view(int view);
	YZLine	*find_line(int line);

	YZLine *line_first, *line_last;
	int	lines; // number of lines in this buffer

	/* readonly?, change, load, save, isclean?, ... */
	/* locking stuff will be here, too */
};

#endif // __cplusplus



/*
 * C API
 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


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

/** load the file (from 'path') */
void buffer_load(yz_buffer);
/** save the file (in 'path', must be set) */
void buffer_save(yz_buffer);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif //  YZ_BUFFER_H

