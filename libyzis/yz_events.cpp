/**
 * $Id$
 */

#include "yz_events.h"

YZEvent::YZEvent() {
}

yz_event YZEvent::mkEventStatus(int view, const QString& text) {
	yz_event e;
	e.id=YZ_EV_SET_STATUS;
	e.setstatus.text=text;
	e.view=view;
	return e;
}


yz_event YZEvent::mkEventCursor(int view, int x, int y, int y2, const QString& p) {
	yz_event e;
	e.id=YZ_EV_SET_CURSOR;
	e.setcursor.x=x;
	e.setcursor.y=y;
	e.setcursor.y2=y2;
	e.setcursor.percentage=p;
	e.view = view;
	return e;
}

yz_event YZEvent::mkEventInvalidateLine(int view,int l) {
	yz_event e;
	e.id= YZ_EV_INVALIDATE_LINE;
	e.invalidateline.y=l;
	e.view = view;
	return e;
}

yz_event YZEvent::mkEventRedraw(int view) {
	yz_event e;
	e.id= YZ_EV_REDRAW;
	e.view = view;
	return e;
}

yz_event YZEvent::mkEventNoop() {
	yz_event e;
	e.id=YZ_EV_NOOP;
	return e;
}

