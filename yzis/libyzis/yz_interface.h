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


	avant de definir les lignes et les colonnes, il faut decider ce qu'on fait des lignes trop longues (voir binaires)

	1) on definit un max, genre 1024, et on prevoit un moyen pour dire que en fait c'est plus long (et le gui affiche "..." ou '@' ou ce qu'il veut)
	2) on definit pas de max, et on se gere un allocateur de memoire complique ds le core, why not

	sinon, en gros, ce que je vois c'est un 
	typedef struct { unicode_char_t letter, int color } yz_char;
	/* avec le color definit par rapport a une table de couleur, changeable bien sur, etc... */
	puis un 
	typedef yz_line yz_char[MAX_LINE_LEN];

	et yz_line est la chose basique qu'on s'echange avec le core
	bon d'accord, sizeof(yz_line) doit etre du genre MAX_LINE_LEN*8. ce qui fait bien 400 k pour MAX=1024 et 50 lignes, mais bon ...



/**
 * Yzis events
 * (yet to be defined)
 */

enum yz_events {
	YZ_EV_asdf,
	YZ_EV_asdf2,
	YZ_EV_asdf3,
	YZ_EV_asdf4
};


/** here we can use long name as we shouldn't need to use those anyway */
struct yz_event_asdf {
	int a;
	float b;
};

struct yz_event_asdf2 {
	char c;
};


/* exemple of use : 
 * (e is of type yz_event)
 * switch (e.id) {
 	case YZ_EV_asdf:
		here we use e.u.asdf.a
		here we use e.u.asdf.b
		break;

 	case YZ_EV_asdf2:
   }

 */
struct yz_event {
	enum yz_events id;
	union {
		struct yz_event_asdf asdf;
		struct yz_event_asdf2 asdf2;
	} u;
};

#ifdef __cplusplus
}
#endif


/**
 * here, we should take care of always having the object as first arg, basically, we
 * do c++ in c, and the first arg is 'this'. (that's how c++ is implemented..)
 */

/** yz_buffer - abstraction of a file */

/** creates an empty buffer */
yz_buffer *create_empty_buffer(void);
/** opens a buffer using the given file */
yz_buffer *create_buffer(char * /*...*/);


/** yz_view - abstraction of a view. binded to a buffer of course */
yz_view *create_view(yz_buffer *, /*...*/);
void yz_send_char(yz_view *, /* ?*/);
void yz_fetch_event(yz_view *, struct yz_event * /*, ... */);
/**
  * this returns the geometry of this yz_view, that is
  * 	the first line displayed
  * 	the number of line displayed
  */
void yz_get_geometry(yz_view *, int *firstline, int *line_nbr);



#ifdef __cplusplus
/**
 *  Here are the c++ bindings for the previously defined c functions/constants..
 */


//class YZEvent, ? with events beeing herited classes ? mm.. dunno


class YZBuffer {
public:
	/** creates an empty buffer */
	YZBuffer(void);
	/** opens a buffer using the given file */
	YZBuffer(/*char * or ?? */ );

	/* readonly?, change, load, save, isclean?, ... */
	/* locking stuff will be here, too */
};


class YZView {
public:
	enum Events { // mm.. redundants, but cleaner ?
		asdf	= YZ_EV_asdf,
		asdf2	= YZ_EV_asdf2,
		asdf3	= YZ_EV_asdf3,
		asdf4	= YZ_EV_asdf4
	};

	YZView(YZBuffer *);
	void	send_char(/*? */);

	/* for the qt/kde gui, we should create QEvents from that? */
	void	fetch_event(/* asasdfasf */);
	int	get_first_line(void) { int a,b; yz_get_geometry(buffer,&a,&b); return a;}
	int	get_lines_displayed(void) { int a,b; yz_get_geometry(buffer,&a,&b); return b;}

private:
	YZBuffer *buffer; /* or &buffer? */


	/*
	 * mode handling (on a per-view basis, no?)
	 * most of command handling and drawing stuff...
	 * yzview should update the core as to how many lines it can display (changegeometry stuff)
	 */
};

#endif


