/**
 * yz_interface.h
 *
 * exported interfaces by libyzis, to be used by gui implementations
 */


#ifdef __cplusplus
extern "C" {
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

typedef struct { unicode_char_t letter; int color; } yz_char;
/* avec le color definit par rapport a une table de couleur, changeable bien sur, etc... */

typedef struct {
	int	len;
	int	flags;
	yz_char text[YZ_MAX_LINE_LENGTH];
} yz_line;


/**
 * Yzis events
 */

enum yz_events {
	YZ_EV_SETLINE,
	YZ_EV_SETCURSOR,
};

/** here we can use long name as we shouldn't need to use those anyway */
struct yz_event_setline {
	int	line_nb;
	yz_line	*line;
};

struct yz_event_setcursor {
	int x,y;
};


/* exemple of use : 
 * (e is of type yz_event)
 * switch (e.id) {
	 case YZ_EV_SETLINE:
		here we use e.u.setline.line_nb;
		here we use e.u.setline.line;
		break;

	case YZ_EV_SETCURSOR:
   }

 */
typedef struct {
	enum yz_events id;
	union {
		struct yz_event_setline;
		struct yz_event_setcursor;
	} u;
} yz_event;


/**
 * here, we should take care of always having the object as first arg, basically, we
 * do c++ in c, and the first arg is 'this'. (that's how c++ is implemented..)
 */

/** yz_buffer - abstraction of a file */
typedef struct {
	char *path; /** path of the file, NULL if no file */
} yz_buffer;

/** creates an empty buffer */
yz_buffer *create_empty_buffer(void);
/** opens a buffer using the given file */
yz_buffer *create_buffer(char *path);

/** add a character on the (x,y) position, move all the line to right */
void buffer_addchar(yz_buffer , int x, int y, );
/** change the character on the (x,y) position, hence erasing the current one */
void buffer_chgchar(yz_buffer , int x, int y, );


/** yz_view - abstraction of a view. binded to a buffer of course */
typedef struct {
	yz_buffer	* buffer;	/** buffer this view is on */
	int		lines;		/** number of lines we can display */
	int		current;	/** current line on top of the view */
	int		x,y;		/** cursor position, beginning at (0,0), not (1,1)... */
	enum {
		YZ_VIEW_MODE_INSERT,
		YZ_VIEW_MODE_REPLACE,
		YZ_VIEW_MODE_COMMAND
	}		mode;

} yz_view;

/**
 * constructor. Each view is binded to a buffer, @param lines is the initial number of lines that
 * this view can display
 */
yz_view *create_view(yz_buffer *, int lines);

/**
 * transfer a key event from gui to core
 */
void yz_send_char(yz_view *, char a);

/**
 * get a event to handle from the core.
 * that's the way the core is sending messages to the gui
 *
 * returns the next event to be handled, or NULL if none
 */
yz_event * yz_fetch_event(yz_view *);

/**
  * this returns the geometry of this yz_view, that is
  * 	the first line displayed
  * 	the number of line displayed
  */
void yz_get_geometry(yz_view *, int *current, int *lines);



#ifdef __cplusplus
}
#endif


/**
 *  Here are the c++ bindings for the previously defined c functions/constants..
 */
#ifdef __cplusplus

//class YZEvent, ? with events beeing herited classes ? mm.. dunno


class YZBuffer {

	friend class YZView;
public:
	/** creates an empty buffer */
	YZBuffer(void) { buffer =  create_empty_buffer(); }
	/** opens a buffer using the given file */
	YZBuffer(char *path) { buffer =  create_buffer(path); }

	/* readonly?, change, load, save, isclean?, ... */
	/* locking stuff will be here, too */
protected:
	yz_buffer * buffer;
};


class YZView {
public:
	enum Events { // mm.. redundants, but cleaner ?
		asdf	= YZ_EV_asdf,
		asdf2	= YZ_EV_asdf2,
		asdf3	= YZ_EV_asdf3,
		asdf4	= YZ_EV_asdf4
	};

	/**
	  * constructor. Each view is binded to a buffer, @param lines is the initial number of lines that
	  * this view can display
	  */
	YZView(YZBuffer *b, int lines) { buffer=b; view = create_view(buffer->buffer, lines); }

	/**
	 * transfer a key event from gui to core
	 */
	void	send_char( char a ) { yz_send_char(view,a); }

	/**
	 * get a event to handle from the core.
	 * that's the way the core is sending messages to the gui
	 */
	/* for the qt/kde gui, we should create QEvents from that? */
	void	fetch_event(/* asasdfasf */);

	/**
	  * returns the number of the line displayed on top of this view
	  * (refering to the whole file/buffer
	  */
	int	get_current(void) { int a,b; yz_get_geometry(view,&a,&b); return a;}

	/**
	  * returns the number of line this view can display
	  */
	int	get_lines_displayed(void) { int a,b; yz_get_geometry(view,&a,&b); return b;}

protected:
	YZBuffer 	*buffer;
	yz_view		*view;


	/*
	 * mode handling (on a per-view basis, no?)
	 * most of command handling and drawing stuff...
	 * yzview should update the core as to how many lines it can display (changegeometry stuff)
	 */
};

/* End of c++ bindings */
#endif


