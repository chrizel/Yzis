#ifndef YZ_LINE_H
#define YZ_LINE_H
/**
 * yz_line.h
 */

#include "yzis.h"


#ifndef NULL
#define NULL 0
#endif


/**
 * 
 * YZLine are used for the interface between the core and the GUI
 * Using those, the core maintains a local 'window' whithin the gui
 * Maintainance is done through the use of events
 * The gui use this local window to draw/update the part seen by the user
 *
 * One can think of those YZLine as pre-rendered text from the core. It's up
 * to the gui to do the final rendering.
 */

/**
 *  define/check  that ushort/uint are as defined/used on :
 *  http://doc.trolltech.com/3.1/qchar.html is there such a type defined in
 *  libc or curses, or... ?
 */

/**
 *  fyi : there's a /usr/include/unicode.h from 'libunicode' package, from
 *  http://www.gnome.org/ 
 *  it's quite short  ~200 lines
 *  from unicode.h :
 */  

	/* FIXME: assumes 32-bit int.  
	typedef unsigned int unicode_char_t; */
/* ugly, temporary */
#define unicode_char_t char


#define YZ_LINE_MAX_LENGTH	4096
#define YZ_LINE_DEFAULT_LENGTH	2048

/* avec le color definit par rapport a une table de couleur, changeable bien sur, etc... */
typedef struct { unicode_char_t letter; int color; } yz_char;



#ifdef __cplusplus
class YZLine {
public:

	enum {
		YZ_LINE_FLAG_TOO_LONG = 1	/* one bit per flag... */
	};

	YZLine(int size=YZ_LINE_DEFAULT_LENGTH);
	/**
	  * @param line : line who corresponds in the file
	  * @param init : the data to be stored in this YZLine
	  * @param datalen : length of data to be taken from @param init
	  * @param size : initial size of this YZLine, the actual size will
	  * be at least max(@param size, @param datalen)
	  */
	YZLine(int _line, char *init, int datalen, int size=YZ_LINE_DEFAULT_LENGTH);

	~YZLine();

	void write_to(int fd);
	void append(char *data, int len);

	/* linked list handling */
	YZLine * next(void) { return p_next; }
	void	set_next(YZLine *n) { p_next = n; }


	/* editing */
	void add_char (int x, unicode_char_t c);
	void chg_char (int x, unicode_char_t c);

	int	line;
	char	*data;		// actual data
	char	*color;	// not used yet
	int	len;		// current len
protected:
	YZLine * p_next;
	void	expand(int newsize); // expand the internal buffer

	int	len_max;	// len of *data,*color
	int	flags;
};



#endif // __cplusplus



/*
 * C API
 */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


typedef int *yz_line;



#ifdef __cplusplus
}
#endif // __cplusplus


#endif //  YZ_LINE_H

