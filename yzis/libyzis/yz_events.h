#ifndef YZ_EVENTS_H
#define YZ_EVENTS_H
/**
 * view.h
 *
 * yzis events
 *
 */


#ifdef __cplusplus
	class YZLine;
#else
	struct yz_line;
#endif // __cplusplus


enum yz_events {
	YZ_EV_SETLINE,
	YZ_EV_SETCURSOR,
};

/** here we can use long name as we shouldn't need to use those anyway */
struct yz_event_setline {
	int	line_nb;
#ifdef __cplusplus
	YZLine 	*line;
#else
	yz_line	*line;
#endif // __cplusplus
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




#endif //  YZ_EVENTS_H

