#ifndef YZ_LINE_H
#define YZ_LINE_H
/**
 * line.h
 */



#ifndef NULL
#define NULL 0
#endif


/**
 * 
 * This is the local 'window' which the core maintains through events, and 
 * which is used by the gui to draw/update the part seen by the user
 *
 */

// define/check  that ushort/uint are as defined/used on : http://doc.trolltech.com/3.1/qchar.html
// is there such a type defined in libc or curses, or... ?

// fyi : there's a /usr/include/unicode.h from 'libunicode' package, from http://www.gnome.org/ 
// it's quite short  ~200 lines
// from unicode.h :

	/* FIXME: assumes 32-bit int.  */
	typedef unsigned int unicode_char_t;


#define YZ_MAX_LINE_LENGTH	2048

/* avec le color definit par rapport a une table de couleur, changeable bien sur, etc... */
typedef struct { unicode_char_t letter; int color; } yz_char;



#ifdef __cplusplus
class YZLine {
public:
	YZLine(void) {len=0; flags=0; }

protected:
	int	len;
	int	flags;
	yz_char text[YZ_MAX_LINE_LENGTH];
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

