#ifndef YZ_EVENTS_H
#define YZ_EVENTS_H
/**
 * $Id$
 *
 * yzis events
 *
 */

#include "yzis.h" /* NULL */
#include <qstring.h>

class QString;

/** list of all events */
enum yz_events {
	YZ_EV_INVALIDATE_LINE,
	YZ_EV_SET_CURSOR,
	YZ_EV_SET_STATUS,
	YZ_EV_REDRAW,
	YZ_EV_NOOP //no, nothing :)
};

/**
 * Be careful, when the core uses yz_event_invalidateline, it is of its responsability
 * to handle the cursor position
 * for a whole screen refresh
 * 	yz_event_invalidateline for line 1
 * 	....
 * 	yz_event_invalidateline for line n
 *
 * 	yz_event_setcursor(wherever)
 *
 * for a modified line
 * 	yz_event_invalidateline for line y
 * 	yz_event_setcursor(wherever)
 *
 * There will be some special events for that, but not yet
 */
struct yz_event_invalidateline {
	int	y;
	yz_event_invalidateline() {
		y=0;
	}
	yz_event_invalidateline(const yz_event_invalidateline& e) {
		y=e.y;
	}
};

/**
 * x and y are NOT relative to the view
 */
struct yz_event_setcursor {
	int x,y,y2;
	QString percentage;
/*	yz_event_setcursor() {
		percentage=QString::null;
	}
	yz_event_setcursor(const yz_event_setcursor& e) {
		percentage=e.percentage;
	}*/
};

struct yz_event_setstatus {
	QString text;
	yz_event_setstatus() {
		text=QString::null;
	}
	yz_event_setstatus(const yz_event_setstatus& e) {
		text=e.text;
	}
};

/**
 * The yz_event struct, mainly an union of all event args
 *
 * exemple of use : 
 * (e is of type yz_event)
 * switch (e.id) {
 case YZ_EV_SETLINE:
 here we use e.u.invalidateline.line_nb;
 break;

 case YZ_EV_SETCURSOR:
 }
 */

struct yz_event_t {
	enum yz_events		id;
	struct yz_event_t	*next;
	struct yz_event_invalidateline		invalidateline;
	struct yz_event_setcursor	setcursor;
	struct yz_event_setstatus	setstatus;
	/* nothing for REDRAW */
};

typedef struct yz_event_t yz_event;

class YZEvent {
	public:
		YZEvent();

		static yz_event mkEventStatus(const QString&);
		static yz_event mkEventCursor(int x, int y, int y2, const QString&);
		static yz_event mkEventInvalidateLine(int);
		static yz_event mkEventRedraw();
		static yz_event mkEventNoop();
};

#endif /*  YZ_EVENTS_H */

