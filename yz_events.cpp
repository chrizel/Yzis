/**
 * yz_event.cpp
 *
 * yzis events
 *
 */


#include "yz_events.h"


yz_event mk_event_setstatus(char *text) { yz_event e; e.id=YZ_EV_SETSTATUS; e.u.setstatus.text=text; return e; }


