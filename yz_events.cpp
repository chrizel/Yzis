/**
 * $Id$
 */

#include "yz_events.h"

YZEvent::YZEvent() {
}

yz_event YZEvent::mkEventStatus(const QString& text) {
	yz_event e;
	e.id=YZ_EV_SETSTATUS;
	e.setstatus.text=text;
	return e;
}

yz_event YZEvent::mkEventCursor(int x, int y) {
	yz_event e;
	e.id=YZ_EV_SETCURSOR;
	e.setcursor.x=x;
	e.setcursor.y=y;
	return e;
}

yz_event YZEvent::mkEventLine(int l, const QString& text) {
	yz_event e;
	e.id=YZ_EV_SETLINE;
	e.setline.y=l;
	e.setline.line=text;
	return e;
}

yz_event YZEvent::mkEventNoop() {
	yz_event e;
	e.id=YZ_EV_NOOP;
	return e;
}

