/**
 * yz_event.cpp
 *
 * yzis events
 *
 */

#include "yz_events.h"

yz_event mk_event_setstatus(const char *text) {
	yz_event e;
	e.id=YZ_EV_SETSTATUS;
	e.u.setstatus.text=text;
	return e;
}

yz_event mk_event_setcursor(int x, int y) {
	yz_event e;
	e.id=YZ_EV_SETCURSOR;
	e.u.setcursor.x=x;
	e.u.setcursor.y=y;
	return e;
}

yz_event mk_event_setline(int l, QString *text) {
	yz_event e;
	e.id=YZ_EV_SETLINE;
	e.u.setline.y=l;
	e.u.setline.line=new QString( *text );
	return e;
}


